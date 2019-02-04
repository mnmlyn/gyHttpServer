/*
 * 连接类，用于表示一个TCP连接
 *
*/

#include "Connection.h"

Connection::Connection(int fd):
            _fd(fd),
            _buffer_size(4096),
            _isConnected(true){
    _buffer = new uint8_t[_buffer_size];
    _buffer_tail = _buffer + _buffer_size;
    _buffer_start = _buffer_end = _buffer;
}

Connection::~Connection(){
    delete[] _buffer;
    this->close();
}

void Connection::close(){
    if(_isConnected){
        ::close(_fd);
        _isConnected = false;
    }
}

void Connection::getRequest(){
    if(!_isConnected)
        return;
    ssize_t n;
    n = read(_fd, _buffer_end, _buffer_tail - _buffer_end);
    if(0 == n){
        this->close();
        return;
    }
    if(n < 0){
        std::cout << "read error\n";
        return;
    }
    _buffer_end += n;
    *_buffer_end = '\0';
    printf("%s\n", _buffer_start);
    char aa[] = "Hello";
    write(_fd, aa, strlen(aa));
    
}












