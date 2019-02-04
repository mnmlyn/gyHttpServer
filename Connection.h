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

#include<iostream>//temp
#include<cstring>
#include<cstdio>

class Connection{
public:
    Connection(int fd);
    ~Connection();
    void close();
    void getRequest();        
private:
    int _fd;
    uint8_t *_buffer;
    uint8_t *_buffer_start;
    uint8_t *_buffer_end;
    uint8_t *_buffer_tail;
    size_t _buffer_size;
    bool _isConnected;
};

#endif
