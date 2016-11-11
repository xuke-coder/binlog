/*
*auther:kellanxu@tencent.com
*date  :20160829
*/


#ifndef BINLOG_H
#define BINLOG_H

#include <proto/pb_file_iterator.h>
#include <proto/proto_reflection.hpp>
#include <proto/varint32.h>

class binlog_file;

class binlog
{
    public:
        binlog(std::string path = folder_path);
        virtual ~binlog();    
        int read(::google::protobuf::Message& value);
        int write(const ::google::protobuf::Message& value);

    private:
        binlog_file    *m_binlog;
        std::string     m_folder_path;
        time_t          m_last_time;
        time_t          m_first_write_time;

};

#endif
