/*
*auther:kellanxu@tencent.com
*date  :20160829
*/


#ifndef BINLOG_FILE_H
#define BINLOG_FILE_H

#include "binlog_status.h"



#define cache_buffer_empty(X) (!X.is_full && X.current == X.last)
#define cache_buffer_full(X) (X.is_full && X.current == X.last)

struct cache_buffer_st
{
    char        *buffer;
    uint64_t    buffer_size;
    char        *start;
    char        *end;
    char        *current;
    char        *last;
    int         is_full;
};




class binlog_file
{
    public:
        binlog_file();
        virtual ~binlog_file();

        int binlog_read(char *buf, uint64_t buf_size);
        int binlog_write(char *buf, uint64_t buf_size);

    
        virtual int cache_buffer_init(uint64_t buffer_size);
        virtual void cache_buffer_release();
        virtual int cache_buffer_swap(char *buf, uint32_t need) = 0;
        virtual int cache_buffer_io() = 0;

    public:
        std::string     m_file_path;
        binlog_status   m_status;
        uint32_t        m_fd;
        cache_buffer_st  m_buf;
        
};

#endif
