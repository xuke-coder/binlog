/*
*auther:kellanxu@tencent.com
*date  :20160829
*/


#ifndef BINLOG_WRITE_FILE_H
#define BINLOG_WRITE_FILE_H

#include "binlog_status.h"

class binlog_write_file : public binlog_file {
    public:
        binlog_write_file();
        virtual ~binlog_write_file();
        static binlog_file* new_binlog(std::string path, uint64_t buf_size = DEFAULT_BUF_SIZE);
        virtual int cache_buffer_swap(char *buf, uint32_t need);
        virtual int cache_buffer_io();
        void flush_to_disk();
    private:
        time_t m_last_write;
};

#endif
