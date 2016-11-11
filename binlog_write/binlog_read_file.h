/*
*auther:kellanxu@tencent.com
*date  :20160829
*/


#ifndef BINLOG_READ_FILE_H
#define BINLOG_READ_FILE_H

class binlog_read_file : public binlog_file {
    public:
        binlog_read_file();
        static binlog_file* new_binlog(std::string &path, uint64_t buf_size = DEFAULT_BUF_SIZE);
        virtual int cache_buffer_swap(char *out, uint32_t need);
        virtual int cache_buffer_io();

    private:
        int m_status_fd;
        time_t m_last_read_failed;
        pid_t m_tid;
        uint64_t m_offset;
};

#endif
