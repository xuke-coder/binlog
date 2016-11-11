#include <vector>
#include <string>
#include "binlog_util.h"
#include "binlog.h"
#include "tl_filter.pb.h"

int main()
{
    binlog *bl = new binlog();
    if (!bl) {
        printf("binlog getInstance error!");
        return -1;
    }

    tl_filter::TlRequest req;

    char temp[100] = {0};
    
    int n = 1000;
    while (n > 0) {
        req.Clear();
        sprintf(temp, "%d", n);
        tl_filter::Header *header = req.mutable_header();
        header->set_seq_id(11);
        header->set_cmd(tl_filter::SYNC_TIMELINE);
        tl_filter::TlFilter *filter = req.add_tl_filter();

        tl_filter::UserInfo *userInfo = filter->mutable_userinfo();
        userInfo->set_uin(0);
        userInfo->set_deviceid("aaa");
        userInfo->set_openid("bbb");
        userInfo->set_omgid("ccc");
        std::string doc1(temp);
        std::string *docs = filter->add_docs();
        *docs = doc1;
        
        int ret = bl->write(req);
        if (ret != BINLOG_OK) {
            printf("binlog write error!");
        }
        n--;
        printf("n=%d\n", n);

        usleep(20 * 1000);
    }

    delete bl;

    return 0;

    
}
