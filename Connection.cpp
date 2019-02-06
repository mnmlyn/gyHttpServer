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
    if (_status == INIT)
        _status = REQUEST_LINE;
    if (!_request)
        _request = new HttpRequest();
    parseHttpRequest();
    if (!_request->is_finish)
        getRequest();
    if (_request->is_finish){
        sendResponse();
        _request->is_finish = false;
    }
}

void Connection::sendResponse() {
    int fp, content_length;
    if (_status != INIT || !_isConnected || !_request) {
        LOG(DEBUG) << "Can not send Response\n";
        return;
    }
    LOG(DEBUG) << "prepare response\n";
    std::string ss("");
    ss += "HTTP/1.1 200 OK\r\n";
    if (_request->url == "/") {
        std::string html("<html><head><title>gyHttpServer</title></head><body><img src=\"/shit.jpg\"  alt=\"this is a shit\"/><p>hello http server!</p></body></html>");
        ss += "Content-Type: text/html\r\n";
        ss += "Content-Length: " + std::to_string(html.size()) + "\r\n";
        ss += "\r\n" + html;
        LOG(DEBUG) << "url is /, send response\n";
        write(_fd, ss.c_str(), ss.size());
    }
    else if (_request->url == "/favicon.ico" 
            || _request->url == "/shit.jpg") {
        FILE *filep;
        if (_request->url == "/favicon.ico")
            filep = fopen("favicon.ico", "rb");
        else if (_request->url == "/shit.jpg")
            filep = fopen("shit.jpg", "rb");
        fseek(filep, 0, SEEK_END);
        content_length = ftell(filep);
        fclose(filep);
        if (_request->url == "/favicon.ico")
            ss += "Content-Type: image/x-icon\r\n";
        else if (_request->url == "/shit.jpg")
            ss += "Content-Type: image/jpeg\r\n";
        ss += "Content-Length: " + std::to_string(content_length) + "\r\n" + "\r\n";
        write(_fd, ss.c_str(), ss.size());
        
        if (_request->url == "/favicon.ico")
            fp = open("favicon.ico", O_RDONLY);
        else if (_request->url == "/shit.jpg")
            fp = open("shit.jpg", O_RDONLY);
        uint8_t buff[128];
        ssize_t n;
        while (n = read(fp, buff, 128)) {
            write(_fd, buff, n);
        }
        ::close(fp);
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
        if (len == WAIT_RECV_MORE)
            break;
        else if (len == PARSE_ERR) {
            LOG(ERROR) << "discard spase after GET err\n";
            is_succ = false;
            break;
        }

        //url
        len = count_until_chr_in_line(p, ' ', n);
        if (len == WAIT_RECV_MORE)
            break;
        else if (len == PARSE_ERR) {
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
        if (len == WAIT_RECV_MORE)
            break;
        else if (len == PARSE_ERR) {
            is_succ = false;
            LOG(ERROR) << "version discard err\n";
            break;
        }
        
        _status = HEADER;
        _buffer_start = p;
    }
    
    if (_status == HEADER) {
        LOG(TRACE) << "parsing _status==HEADER\n";
        while (len = discard_line(p, n)) {
            if (len == PARSE_ERR) {
                is_succ = false;
                LOG(ERROR) << "header discard err\n";
                break;
            }
            else if (len == WAIT_RECV_MORE){
                LOG(TRACE) << "header wait recv\n";
                break;
            }
            _buffer_start = p;
        }
        if (len == 0) {
            LOG(TRACE) << "header finish\n";
            _status = BODY;
            _buffer_start = p;
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
        _buffer_start = p;
        _request->is_finish = true;
        LOG(TRACE) << "parse OK! _status==INIT\n";
    }

    }while(0);
    if (!is_succ) {
        this->close();
        LOG(ERROR) << "prase err, close\n";
    }
    n = _buffer_end - _buffer_start;
    memmove(_buffer, _buffer_start, n);
    _buffer_start = _buffer;
    _buffer_end = _buffer + n;
    LOG(DEBUG) << "memmove()" << _buffer_end - _buffer_start << std::endl << n << std::endl;
}











