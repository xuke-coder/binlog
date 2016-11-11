/*
*auther:kellanxu@tencent.com
*date  :20160829
*/


#include <unistd.h>
#include <string>
#include "log_def.h"
#include "binlog_util.h"
#include "binlog.h"
#include "binlog_file.h"
#include "binlog_write_file.h"
#include "binlog_read_file.h"


binlog::binlog(std::string path)
{
    m_folder_path = path;
    m_binlog = NULL;
}

binlog::~binlog()
{
    if (m_binlog) {
        delete m_binlog;
    }
}


int
binlog::write(const ::google::protobuf::Message& value)
{
    int ret;
    
    time_t now = time(NULL);
    
    if (!m_binlog) {
        m_first_write_time = now;

        m_binlog = binlog_write_file::new_binlog(m_folder_path);
        if (!m_binlog) {
            Printf("new binlog error!\n");
            return BINLOG_ERROR;
        }

    //超过10分钟没有数据写入， 
    // 换一个新文件写，因为读端15分钟没有新内容读取，会换另一个文件读
    //一天换一个文件
    } else if (m_last_time - now >= 600 || m_first_write_time + 86400 < now) {
        delete m_binlog;
        m_binlog = binlog_write_file::new_binlog(m_folder_path);
        if (!m_binlog) {
            Printf("new binlog error!");
            return BINLOG_ERROR;
        }
    }
    
    
    std::string message;
    if (!value.SerializeToString(&message)) {
        Printf("serialize to string error");
        return BINLOG_ERROR;
    }

    ret = m_binlog->cache_buffer_swap(const_cast<char *>(message.c_str()), message.length());

    m_last_time = now;

    return ret;
}

int
binlog::read(::google::protobuf::Message& value)
{
    int ret;

    while (1) {
        time_t now = time(NULL);

        if (!m_binlog) {
            m_binlog = binlog_read_file::new_binlog(m_folder_path);
            if (!m_binlog) {
                Printf("new binlog error!\n");
                return BINLOG_ERROR;
            }
        }

        uint32_t len;

        ret = m_binlog->cache_buffer_swap((char *)&len, 4);
        if (ret == BINLOG_ERROR) {
            Printf("read len cache_buffer_swap error!\n");
            delete m_binlog;
            m_binlog = NULL;
            continue;
        }

        std::string message;
        message.resize(len);
        
        ret = m_binlog->cache_buffer_swap(&message[0], len);
        if (ret == BINLOG_ERROR) {
            Printf("read message cache_buffer_swap error!\n");
            delete m_binlog;
            m_binlog = NULL;
            continue;
        }
        
        m_last_time = now;

        value.ParseFromString(message);
        break;
    }

    return BINLOG_OK;
}


