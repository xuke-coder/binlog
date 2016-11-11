#include <vector>
#include <string>
#include "log_def.h"
#include "common_util.h"
#include "binlog_util.h"
#include "binlog.h"
#include "tl_filter.pb.h"

int exit1;
int exit2;

void
run();
void*
worker(void *ptr);

int main()
{
    run();
    return 0;
    
}

void
run()
{
    int n = 2;
    pthread_t pid;
    std::vector<pthread_t> pid_list;
    std::vector<pthread_t>::iterator iter;
    
    for (int i = 0; i < n; i++) {
        pthread_create(&pid, NULL, worker, NULL);
        Printf("pid=%d\n", pid);
        pid_list.push_back(pid);
    }

    for (iter = pid_list.begin(); iter != pid_list.end(); iter++) {
        void *a;
        pthread_join(*iter, &a);
    }
}

void*
worker(void *ptr)
{
    binlog *bl = new binlog();
    Printf("now...");

    tl_filter::TlRequest req;

    int n = 1000;
    
    while (n > 0) {
        int ret = bl->read(req);
        if (ret != BINLOG_OK) {
            Printf("binlog read error!\n");
        } else {
            Printf("read ok \n");
            //P_PROTO("req:", req);
        }
        n--;
    }

    pthread_exit(NULL);

    return NULL;
    
}
