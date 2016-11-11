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
#include "log_def.h"


binlog_file::binlog_file()
{
    m_fd = 0;
    memset(&m_buf, 0, sizeof(m_buf));
}

binlog_file::~binlog_file()
{
    if (m_buf.buffer) {
        cache_buffer_release();
    }

    if (m_fd) {
        close(m_fd);
    }
}


int
binlog_file::binlog_read(char *buf, uint64_t buf_size)
{
    uint64_t length;
    return cache_buffer_swap(buf, buf_size);
}

int
binlog_file::binlog_write(char *buf, uint64_t buf_size)
{
    return cache_buffer_swap(buf, buf_size);
}


int
binlog_file::cache_buffer_init(uint64_t buffer_size)
{
    m_buf.buffer = (char *)malloc(buffer_size);
    if (!m_buf.buffer) {
        Printf("malloc error!");
        return BINLOG_ERROR;
    }

    m_buf.buffer_size = buffer_size;
    m_buf.start = m_buf.buffer;
    m_buf.end = m_buf.buffer + buffer_size;
    m_buf.current = m_buf.start;
    m_buf.last = m_buf.start;
    m_buf.is_full = BINLOG_FALSE;

    return BINLOG_OK;
}

void
binlog_file::cache_buffer_release()
{
    free(m_buf.buffer);
    memset(&m_buf, 0, sizeof(m_buf));
}

    

