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
        LOG(TRACE) << "Connection close," << _fd << "\n";
        return;
    }
    if(n < 0){
        LOG(ERROR) << "read error\n";
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
    if (_status != INIT || !_isConnected || !_request) {
        LOG(DEBUG) << "Can not send Response\n";
        return;
    }
    LOG(DEBUG) << "prepare response\n";
    std::string html("<html><head><title>gyHttpServer</title></head><body><p>hello http server!</p></body></html>");
    std::string ss("");
    ss += "HTTP/1.1 200 OK\r\n";
    ss += "Content-Type: text/html;charset=ISO-8859-1\r\n";
    ss += "Content-Length: " + std::to_string(html.size()) + "\r\n";
    ss += "\r\n" + html;
    if (_request->url == "/") {
        LOG(DEBUG) << "url is /, send response\n";
        write(_fd, ss.c_str(), ss.size());
    }
    else{
        LOG(DEBUG) << "url not /, no response\n";
        this->close();
    }
}

void Connection::parseHttpRequest() {
    size_t n = _buffer_end - _buffer_start;
    uint8_t *p = _buffer_start;
    bool is_succ = true;
    size_t len;
    do {
    if (_status == REQUEST_LINE) {
        LOG(TRACE) << "parsing _status==REQUEST_LINE\n";
        if (n < 16)//GET / HTTP/1.1\r\n
            break;
        if (memcmp(p, "GET", 3) != 0) {
            LOG(WARNING) << "not GET\n";
            is_succ = false;
            break;
        }
        p += 3;
        n -= 3;
        _request->method = HttpRequest::GET;
        LOG(DEBUG) << "GET\n";

        len = discard_chr(p, ' ', n);
        if (len == -1)
            break;
        else if (len == -2) {
            LOG(ERROR) << "discard spase after GET err\n";
            is_succ = false;
            break;
        }

        //url
        len = count_until_chr_in_line(p, ' ', n);
        if (len == -1)
            break;
        else if (len == -2) {
            LOG(ERROR) << "url spase err\n";
            is_succ = false;
            break;
        }

        _request->url = std::move(
                std::string((char *)p, len));
        p += len;
        n -= len;
        LOG(TRACE) << _request->url << std::endl;
        
        //version discard
        len = discard_line(p, n);
        if (len == -1)
            break;
        else if (len == -2) {
            is_succ = false;
            LOG(ERROR) << "version discard err\n";
            break;
        }
        
        _status = HEADER;
    }
    
    if (_status == HEADER) {
        LOG(TRACE) << "parsing _status==HEADER\n";
        while (len = discard_line(p, n)) {
            if (len == -2) {
                is_succ = false;
                LOG(ERROR) << "header discard err\n";
                break;
            }
            else if (len == -1){
                LOG(TRACE) << "header wait recv\n";
                break;
            }
        }
        if (len == 0) {
            LOG(TRACE) << "header finish\n";
            _status = BODY;
        }
        if (len < 0){
            LOG(TRACE) << "header len<0 err\n";
            break;
        }
    }

    //now, no body
    if (_status == BODY){
        LOG(TRACE) << "parsing _status==BODY\n";
        _status = INIT;
        LOG(TRACE) << "parse OK! _status==INIT\n";
    }

    }while(0);
    if (!is_succ) {
        this->close();
        LOG(ERROR) << "prase err, close\n";
    }
}











