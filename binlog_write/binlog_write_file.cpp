/*
*auther:kellanxu@tencent.com
*date  :20160829
*/




#include <vector>

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "log_def.h"
#include "binlog_util.h"
#include "binlog_file.h"
#include "binlog_write_file.h"
    

binlog_write_file::binlog_write_file()
{
    m_last_write = 0;
}

binlog_write_file::~binlog_write_file()
{
    Printf("clear the buffer\n");
    cache_buffer_io();
    flush_to_disk();
}



binlog_file* 
binlog_write_file::new_binlog(std::string path, uint64_t buf_size)
{
    time_t now = time(NULL);
    
    if (buf_size <= 0) {
        Printf("buf_size=%ld", buf_size);
        return NULL;
    }

    if (buf_size < 1024) {
        buf_size = DEFAULT_BUF_SIZE;
    }

    binlog_file *binlog = new binlog_write_file();

    binlog->m_file_path = path + binlog->m_status.get_file_name(now);

    if (binlog->cache_buffer_init(buf_size) != BINLOG_OK) {
        delete binlog;
        return NULL;
    }

    binlog->m_fd = open(binlog->m_file_path.c_str(), O_WRONLY | O_APPEND |  O_CREAT, 0666);

    if (binlog->m_fd < 0) {
        Printf("open binlog error!");
        delete binlog;
        return NULL;
    }

    return binlog;
}

int
binlog_write_file::cache_buffer_swap(char *buf, uint32_t need)
{
    uint32_t    len = 0;
    int         ret;
    char       *pos;
    uint32_t    need_write = need + 4;//前4个字节是长度

    time_t now = time(NULL);

    if (m_last_write == 0) {
        m_last_write = now;
    }

    if (m_buf.end - m_buf.last >= need_write) {
        memcpy(m_buf.last, &need, sizeof(need));
        memcpy(m_buf.last + 4, buf, need);
        m_buf.last += need_write;
        if (m_buf.last == m_buf.end) {
            m_buf.last = m_buf.start;
        }
    } else {
        //not enough space to cache
        cache_buffer_io();
        if (m_buf.end - m_buf.last >= need_write) {
            memcpy(m_buf.last, &need, sizeof(need));
            memcpy(m_buf.last + 4, buf, need);
            m_buf.last += need_write;
            if (m_buf.last == m_buf.end) {
                m_buf.last = m_buf.start;
            }
        }
    }

    //if buf is full , write to disk
    if (m_buf.last == m_buf.start) {
        m_buf.is_full = BINLOG_TRUE;
        cache_buffer_io();
        m_last_write = now;

    //超过1秒就write一次
    } else {
        if (now - m_last_write >= 1) {
            cache_buffer_io();
            m_last_write = now;
        }
    }

    return BINLOG_OK;
}


int
binlog_write_file::cache_buffer_io()
{
    int ret;
    uint64_t need_write;
    
    if (m_buf.is_full == BINLOG_TRUE) {
        m_buf.is_full = BINLOG_FALSE;        
        need_write = m_buf.buffer_size;
        
    } else {
        need_write = m_buf.last - m_buf.start;
        
    }

    Printf("start :%d, %d", m_buf.start, need_write);
    ret = write(m_fd, m_buf.start, need_write);
    m_buf.last = m_buf.start;
    
    if (ret != need_write) {
        Printf("binlog write error! ret = %d, need_write = %ld, errno=%d\n",
            ret, need_write, errno);
        return BINLOG_ERROR;
    } 
    
    Printf("binlog write sucess! ret = %d, need_write = %ld\n",
            ret, need_write);
    return BINLOG_TRUE;
    
}

void
binlog_write_file::flush_to_disk()
{
    fsync(m_fd);
}
