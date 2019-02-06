/*
 * 连接类，用于表示一个TCP连接
 *
*/

#include "Connection.h"

Connection::Connection(int fd):
            _fd(fd),
            _buffer_size(4096),
            _isConnected(true),
            _request(nullptr),
            _status(INIT) {
    _buffer = new uint8_t[_buffer_size];
    _buffer_tail = _buffer + _buffer_size;
    _buffer_start = _buffer_end = _buffer;
}

Connection::~Connection() {
    if (_request)
        delete _request;
    delete[] _buffer;
    this->close();
}

void Connection::close() {
    if(_isConnected){
        ::close(_fd);
        _isConnected = false;
    }
}

void Connection::getRequest() {
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
    //n > 0
    _buffer_end += n;
    _status = REQUEST_LINE;
    if (!_request)
        _request = new HttpRequest();
    parseHttpRequest();
    sendResponse();
}

void Connection::sendResponse() {
    if (_status == INIT)std::cout << "INIT";
    if (_status == REQUEST_LINE)std::cout << "REQUEST_LINE";
    if (_status == HEADER)std::cout << "HEADER";
    if (_status == BODY)std::cout << "BODY";
    std::cout << std::endl;
    if (_status != INIT || !_isConnected || !_request)
        return;
    std::cout << "prepare response\n";
    std::string html("<html><head><title>gyHttpServer</title></head><body><p>hello http server!</p></body></html>");
    std::string ss("");
    ss += "HTTP/1.1 200 OK\r\n";
    ss += "Content-Type: text/html;charset=ISO-8859-1\r\n";
    ss += "Content-Length: " + std::to_string(html.size()) + "\r\n";
    ss += "\r\n" + html;
    if (_request->url == "/") {
        std::cout << "url is /, send response\n";
        write(_fd, ss.c_str(), ss.size());
    }
    else{
        std::cout << "url not /, no response\n";
        this->close();
    }
}

size_t line_until_chr(const uint8_t * str, char ch1,
        size_t max) {
    size_t i;
    for (i = 0; i < max; ++i) {
        if (str[i] == ch1)
            return i;
        if (str[i] == '\r'
                || str[i] == '\n') {
            return -2;//err
        }
    }
    return -1;//wait recv
}

size_t discard_chr(uint8_t *&str, const char &ch, size_t &max) {
    if (max < 1)
        return -1;//wait recv
    if (str[0] != ch) {
        return -2;//err
    }
    ++str;
    --max;
    return 0;
}

size_t mem_line(uint8_t *&str, size_t &max) {
    size_t i;
    if (max < 2)
        return -1;//wait recv
    for (i = 1; i < max; ++i) {
        if (str[i] == '\n' && str[i-1] == '\r') {
            return i - 1;
        }
        if (str[i-1] == '\n' && str[i-1] == '\r') {
            return -2;//err
        }
    }
    return -1;//wait recv
}

size_t discard_line(uint8_t *&str, size_t &max) {
    size_t len = mem_line(str, max);
    printf("discard_line %d\n", (int)len);
    if(len < 0 )
        return len;
    str += len + 2;
    max -= len - 2;
    return len;
}

#include<stdio.h>
void Connection::parseHttpRequest() {
    size_t n = _buffer_end - _buffer_start;
    uint8_t *p = _buffer_start;
    bool is_succ = true;
    size_t len;
    do {
    if (_status == REQUEST_LINE) {
        std::cout << "parseHttpRequest REQUEST_LINE\n";
        if (n < 16)//GET / HTTP/1.1\r\n
            break;
        if (memcmp(p, "GET", 3) != 0) {
            std::cout << "not GET err\n";
            is_succ = false;
            break;
        }
        p += 3;
        n -= 3;
        _request->method = HttpRequest::GET;
        std::cout << "GET\n";

        len = discard_chr(p, ' ', n);
        if (len == -1)
            break;
        else if (len == -2) {
            std::cout << "discard spase after GET err\n";
            is_succ = false;
            break;
        }

        //url
        len = line_until_chr(p, ' ', n);
        if (len == -1)
            break;
        else if (len == -2) {
            std::cout << "url spase err\n";
            is_succ = false;
            break;
        }

        _request->url = std::move(
                std::string((char *)p, len));
        p += len;
        n -= len;
        std::cout << _request->url << std::endl;
        
        //version discard
        len = discard_line(p, n);
        if (len == -1)
            break;
        else if (len == -2) {
            is_succ = false;
            std::cout << "version discard err\n";
            break;
        }
        
        _status = HEADER;
    }
    
    if (_status == HEADER) {
        std::cout << "parseHttpRequest HEADER\n";
        printf("%c\n", p[0]);
        printf("%c\n", p[1]);
        while (len = discard_line(p, n)) {
            if (len == -2) {
                is_succ = false;
                std::cout << "header discard err\n";
                break;
            }
            else if (len == -1){
                std::cout << "header wait recv err\n";
                break;
            }
        }
        if (len == 0) {
            std::cout << "header finish err\n";
            _status = BODY;
        }
        if (len < 0){
            std::cout << "header len<0 err\n";
            break;
        }
    }

    //now, no body
    if (_status == BODY){
        std::cout << "parseHttpRequest BODY\n";
        _status = INIT;
        std::cout << "parse OK!init\n";
    }

    }while(0);
    if (!is_succ) {
        this->close();
        std::cout << "pase err\n";
    }
}











