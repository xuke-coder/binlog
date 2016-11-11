#ifndef BINLOG_UTIL_H
#define BINLOG_UTIL_H

#include <sys/types.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <dirent.h>
#include <sys/file.h>
#include <time.h>



#include <errno.h>
       
#define gettid() syscall(__NR_gettid) 

#define folder_path "/usr/local/services/binlog/"
//#define file_name "binlog"
//#define file_path folder_path##file_name
#define sync_file_name "sync"

enum BINLOG_OPT_TYPE {
    BINLOG_WRITE,
    BINLOG_READ
};

#define BINLOG_TRUE             1
#define BINLOG_FALSE            0
#define BINLOG_OK               0
#define BINLOG_ERROR           -1
#define BINLOG_READ_FINISH      2
#define BINLOG_BUF_EMPTY        3
#define BINLOG_BUF_FULL         4
#define BINLOG_READ_ERROR       BINLOG_ERROR
#define DEFAULT_BUF_SIZE        (10 * 1024 * 1024)

#define P_PROTO(prefix, pb) do{    \
        std::string output; \
        BASE::Print(&output, pb); \
        if(output.length() < (BASE::Log::kLogBufferSize - 512)) \
            Printf("%s\n%s", prefix, output.c_str()); \
        else \        
            Printf("%s\npb too long:%d", prefix, output.length()); \
}while(0)

#define Printf(X,...) do{      \
        printf("%d %s %d:  ", gettid(), __FILE__, __LINE__); \
        printf(X, ##__VA_ARGS__);\
    } while (0)


#endif

