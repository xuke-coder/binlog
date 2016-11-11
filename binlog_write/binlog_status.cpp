
#include <vector>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "binlog_util.h"
#include "log_def.h"
#include "binlog_file.h"
#include "binlog_status.h"

binlog_status::binlog_status()
{
    m_fd = 0;

    sync_file_init();
}

binlog_status::~binlog_status()
{
    sync_file_release();
}


int
binlog_status::set_status_info(int finished, int tid, uint64_t offset)
{
    int fd= open(m_status_name.c_str(), O_RDWR, 0666);
    char buf[100] = {0};
    sprintf(buf, "%d %d %ld", finished, tid, offset);
    int ret = write(fd, buf, strlen(buf));
    Printf("write:%s\n", buf);
    close(fd);
    return BINLOG_OK;
}

int
binlog_status::get_status_info(int &finished, std::string &tid, uint64_t &offset)
{   
    int fd= open(m_status_name.c_str(), O_RDWR, 0666);
    char buf[100] = {0};
    int ret = read(fd, buf, 100);
    Printf("ret=%d, %s\n", ret, buf);
    close(fd);

    finished = 0;
    tid ="0";
    offset = 0;
    
    char *token = " ";
    char *p = strtok(buf, token);
    if (p) {
        finished = atoi(p);
        p = strtok(NULL, token);
        if (p) {
            std::string str_tid(p);
            tid = str_tid;
            p = strtok(NULL, token);
            if (p) {
                offset = atol(p);
            }
        }
    
    }
    Printf("%d, %s, %ld\n", finished, tid.c_str(), offset);
    

    return BINLOG_OK;
}

std::string
binlog_status::get_file_name(uint64_t &off)
{
    int ret;
    std::string read_file("");
    
    std::vector<std::string> bin_list;
    std::vector<std::string> status_list;
    std::vector<std::string>::iterator bin_iter;
    std::vector<std::string>::iterator status_iter;

    
    std::string folder(folder_path);

    //lock
    if (sync_file_lock() != BINLOG_OK) {
        return read_file;
    }
    
    
    get_file_list(bin_list, folder_path, "5");
    get_file_list(status_list, folder_path, "status");
    
    pid_t pid = getpid();
    pid_t curtid = gettid();
    char pidpath[100] = {0};
    sprintf(pidpath, "/proc/%d/task/", pid); 
    std::vector<std::string> tid_list;
    get_file_list(tid_list, pidpath, NULL);
            
    for (bin_iter = bin_list.begin(); bin_iter != bin_list.end(); bin_iter++) {
        int find = 0;
        for (status_iter = status_list.begin(); status_iter != 
            status_list.end() && find == 0; status_iter++) {
            m_status_name= "status" + *bin_iter;
            if (m_status_name == *status_iter) {
                m_status_name = folder + m_status_name;
                //find status, check 
                int finished;
                std::string tid;
                uint64_t offset;
                get_status_info(finished, tid, offset);
                //如果finished值为1，说明已经读完了，找下一个bin文件
                if (finished == 1) {
                    find = 1;
                    continue;

                //finished = 0 ，说明没读完，检查是否有线程在读
                } else {
                    std::vector<std::string>::iterator tid_iter;
                    Printf("xxxx tid=%s\n", tid.c_str());
                    for (tid_iter = tid_list.begin(); tid_iter != tid_list.end();
                        tid_iter++) {
                        //有线程在读
                        if (*tid_iter == tid) {
                            find = 1;
                            continue;
                        }
                    }

                    //没有线程读
                    if (find == 0 && tid_iter == tid_list.end()) {
                        read_file = *bin_iter;
                        set_status_info(finished, curtid, offset);
                        off = offset;
                        sync_file_unlock();
                        return read_file;
                        
                    }
                }
                
            }
        }

        //没有对应的status文件
        if (find == 0 && status_iter == status_list.end()) {
            read_file = *bin_iter;
            m_status_name = folder + "status" + *bin_iter;
            m_fd = open(m_status_name.c_str(), O_CREAT | O_RDWR, 0666);
            if (m_fd < 0) {
                Printf("open status file error:%s", m_status_name.c_str());
                read_file = "";
            }
            set_status_info(0, curtid, 0);
            off = 0;
            sync_file_unlock();
            return read_file;
            
        } 
    }

    //unlock
    sync_file_unlock();
    return read_file;
}

std::string
binlog_status::get_file_name(time_t &time)
{
    char temp[100] = {0};
    sprintf(temp, "5%ld000", time);
    std::string file_name(temp);
    
    sync_file_lock();
    std::string last_name = sync_file_get();
    if (file_name <= last_name) {
        long seq_num = atol(last_name.c_str());
        seq_num++;
        sprintf(temp, "%ld", seq_num);
        std::string new_name(temp);
        file_name = new_name;
        
    }
    sync_file_set(file_name);
    sync_file_unlock();

    return file_name;
    
}


int
binlog_status::get_file_list(std::vector<std::string> &file_list, char *basePath, char *prefix)
{
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return BINLOG_ERROR;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..") == 0) {
            continue;
        }

        if (prefix) {
            if (strncmp(ptr->d_name, prefix, strlen(prefix)) == 0) {
                std::string temp(ptr->d_name);
                file_list.push_back(temp);
            }
        } else {
            std::string temp(ptr->d_name);
            file_list.push_back(temp);
        }
    }

    closedir(dir);

    return BINLOG_OK;
}


int
binlog_status::sync_file_init()
{
    char name[100] = {0};
    sprintf(name, "%s%s", folder_path, sync_file_name);
    m_sync_fd = open(name, O_CREAT | O_RDWR, 0666);
    if (m_sync_fd < 0) {
        Printf("sync file open error!");
        return BINLOG_ERROR;
    }

    return BINLOG_OK;
}

void
binlog_status::sync_file_release()
{
    if (m_sync_fd) {
        close(m_sync_fd);
    }
}

int 
binlog_status::sync_file_lock()
{
    if (flock(m_sync_fd, LOCK_EX) < 0) {
        Printf("%d,sync file lock error\n", m_sync_fd);
        return BINLOG_ERROR;
    }

    return BINLOG_OK;
}

int
binlog_status::sync_file_unlock()
{
    if (flock(m_sync_fd, LOCK_UN) < 0) {
        Printf("%d,sync file unlock error\n", m_sync_fd);
        return BINLOG_ERROR;
    }

    return BINLOG_OK;
}


std::string
binlog_status::sync_file_get()
{
    char c_info[100] = {0};
    read(m_sync_fd, c_info, 100);
    std::string s_info(c_info);
    return s_info;
}


void
binlog_status::sync_file_set(std::string &info)
{
    lseek(m_sync_fd, 0, SEEK_SET);
    int ret = write(m_sync_fd, info.c_str(), info.length());
}

