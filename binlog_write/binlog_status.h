#ifndef BINLOG_STATUS_H
#define BINLOG_STATUS_H


class binlog_status {
    public:
        binlog_status();
        ~binlog_status();
        int set_status_info(int finished, int tid, uint64_t offset);
        int get_status_info(int &finished, std::string &tid, uint64_t &offset);
        std::string get_file_name(time_t &time);
        std::string get_file_name(uint64_t &offset);
        int get_file_list(std::vector<std::string> &file_list, char *basePath, char *prefix);
        int sync_file_init();
        void sync_file_release();
        int sync_file_lock();
        int sync_file_unlock();
        std::string sync_file_get();
        void sync_file_set(std::string &info);
        

    private:
        std::string m_status_name;
        int m_fd;
        int finished;
        int tid;
        int offset;
        int m_sync_fd;
        
};

#endif
