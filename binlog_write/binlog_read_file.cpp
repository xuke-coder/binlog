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
#include "binlog_util.h"
#include "binlog_file.h"
#include "binlog_read_file.h"
#include "log_def.h"




binlog_file* 
binlog_read_file::new_binlog(std::string &path, uint64_t buf_size)
{
    time_t now = time(NULL);
    
    if (buf_size <= 0) {
        Printf("open binlog error:buf_size=%ld\n", buf_size);
        return NULL;
    }

    if (buf_size < 1024) {
        buf_size = DEFAULT_BUF_SIZE;
    }

    binlog_file *binlog = new binlog_read_file();

    uint64_t offset = 0;
    binlog->m_file_path = path + binlog->m_status.get_file_name(offset);
    if (binlog->m_file_path == path) {
        Printf("no binlog need to read!\n");
        usleep(1000 * 1000);//等1秒再读
        delete binlog;
        return NULL;
    }
    Printf("binlog %s\n", binlog->m_file_path.c_str());

    if (binlog->cache_buffer_init(buf_size) != BINLOG_OK) {
        delete binlog;
        return NULL;
    }

    binlog->m_fd = open(binlog->m_file_path.c_str(), O_RDONLY | O_NONBLOCK, 0666);
    if (binlog->m_fd < 0) {
        Printf("open binlog error!\n");
        delete binlog;
        return NULL;
    }
    lseek(binlog->m_fd, offset, SEEK_SET);
    binlog_read_file *read = (binlog_read_file *)binlog;
    read->m_offset = offset;
    Printf("lseek %ld\n", offset);

    return binlog;
}

binlog_read_file::binlog_read_file()
{
    m_status_fd = 0;
    m_last_read_failed = 0;
    m_tid = gettid();
    m_offset = 0;
}

int
binlog_read_file::cache_buffer_swap(char *out, uint32_t need)
{
    uint64_t left = need;
    char *pos = out;
    int ret;

    while (left > 0) {
        if (cache_buffer_empty(m_buf)) {
            ret = cache_buffer_io();
            if (cache_buffer_empty(m_buf)) {
                Printf("cache buffer empty!\n");
                time_t now = time(NULL);
                if (m_last_read_failed) {
                    //超过15分钟没读到信息就结束
                    Printf("elapse..%d\n", now - m_last_read_failed);
                    if (now - m_last_read_failed >= 900) {
                        m_status.set_status_info(1, m_tid, m_offset);
                        return BINLOG_ERROR;

                    //sleep 1 秒，再读
                    } 
                    Printf("sleep\n");
                    usleep(1000 * 1000);
                    continue;
                    
                }

                m_last_read_failed = now;
                continue;
                
            } else {
                m_last_read_failed = 0;
            }
        }

        if (cache_buffer_full(m_buf)) {
            if (m_buf.end - m_buf.current <= left) {
                memcpy(pos, m_buf.current, m_buf.end - m_buf.current);
                left = left - (m_buf.end - m_buf.current);
                pos += (m_buf.end - m_buf.current);
                m_buf.current += (m_buf.end - m_buf.current);
                

            } else {
                memcpy(pos, m_buf.current, left);
                m_buf.current += left;
                pos += left;
                left = 0;
            }
            m_buf.is_full = BINLOG_FALSE;
            
        } else {
            if (m_buf.current < m_buf.last) {
                if (m_buf.last - m_buf.current >= left) {
                    memcpy(pos, m_buf.current, left);
                    pos += left;
                    m_buf.current += left;
                    left = 0;
                } else {
                    memcpy(pos, m_buf.current, m_buf.last - m_buf.current);
                    left -= (m_buf.last - m_buf.current);
                    pos += (m_buf.last - m_buf.current);
                    m_buf.current = m_buf.last;
                }
            } else {
                if (m_buf.end - m_buf.current >= left) {
                    memcpy(pos, m_buf.current, left);
                    pos += left;
                    m_buf.current += left;
                    left = 0;
                } else {
                    memcpy(pos, m_buf.current, m_buf.end - m_buf.current);
                    left -= (m_buf.end - m_buf.current);
                    pos += (m_buf.end - m_buf.current);
                    m_buf.current = m_buf.end;
                }
            }
        }
        
        if (m_buf.current == m_buf.end) {
            m_buf.current = m_buf.start;
        }
    }

    return BINLOG_OK;
}


int
binlog_read_file::cache_buffer_io()
{
    uint64_t need_read;
    int      ret;
    char    *pos;

    if (cache_buffer_full(m_buf)) {
        return BINLOG_OK;
    }

    while (!cache_buffer_full(m_buf)) {

        if (m_buf.last >= m_buf.current) {
            need_read = m_buf.end - m_buf.last;
            pos = m_buf.last;
                
            
        } else {
            need_read = m_buf.current - m_buf.last;
            pos = m_buf.last;
        }

        ret = read(m_fd, pos, need_read);
        Printf("read : %ld, %ld\n", need_read, ret);
        if (ret < 0) {
            return BINLOG_ERROR;
            
        } else if (ret == 0) {
            return BINLOG_READ_FINISH;
            
        } else {
            m_buf.last += ret;
            if (m_buf.last == m_buf.end) {
                m_buf.last = m_buf.start;
            }

            if (m_buf.last == m_buf.current) {
                m_buf.is_full = BINLOG_TRUE;
            }
            m_offset += ret;
            //写到status文件中
            m_status.set_status_info(0, m_tid, m_offset);
        }
    }

    return BINLOG_OK;
}


