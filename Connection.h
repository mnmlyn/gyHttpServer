/*
 * 连接类，表示一个TCP连接
 */

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include<unistd.h>
#include<cstdint>
#include<cstddef>//for size_t
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>

#include<iostream>//temp
#include<cstring>
#include<cstdio>
#include<string>
#include<utility>//for move

#include"logging.h"
#include"http_parse_util.h"

class HttpRequest{
public:
    HttpRequest():is_finish(false){}
public:
    enum METHOD {
        GET,
        POST
    };
    METHOD method;
    std::string url;
    size_t content_length;
    void *data;
    bool is_finish;
};

class Connection{
public:
    Connection(int fd);
    ~Connection();
    void close();
    void getRequest();        
    void sendResponse();
private:
    void parseHttpRequest();
public:
    enum HTTP_PARSE_STATUS {
        INIT,
        REQUEST_LINE,
        HEADER,
        BODY
    };
private:
    int _fd;
    uint8_t *_buffer;//缓冲区起
    uint8_t *_buffer_start;//有效缓冲起
    uint8_t *_buffer_end;//有效缓冲的后一个字节
    uint8_t *_buffer_tail;//缓冲区之后一个字节地址
    size_t _buffer_size;//缓冲区大小
    HttpRequest *_request;
    bool _isConnected;
    HTTP_PARSE_STATUS _status;//当前解析状态
};

#endif
