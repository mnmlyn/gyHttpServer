/*
* 看了好久的书，来动手写一个server吧
* 参考UNP，P140
*/
 
#include<stdio.h>
#include<stdlib.h>//exit
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
// /usr/include/netinet/in.h  
// 在源码的include\uapi\linux\in.h
#include<netinet/in.h>//struct sockaddr_in
#include<unistd.h>//close
#include<string.h>//memset
#include<sys/stat.h>//open
#include<fcntl.h>//open
#include<sys/epoll.h>//epoll_create, epoll_ctl, epoll_wait
#include<memory>//shared_ptr
#include<list>

#include "Connection.h"

#define SERV_PORT 80
#define MAXLINE 4096
#define MAX_EVENTS 10

void
err_exit(const char *err) {
    fprintf(stderr, "Err:%s\n", err);
    exit(1);
}

void setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        err_exit("fcntl failed");
    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if(r < 0)
        err_exit("fcntl failed");
}

int
main(int argc, char **argv) {
    int listenfd, connfd, epfd, nfds, ret, fp, i;
    ssize_t n;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddrlen;
    char buff_r[MAXLINE], buff_s[MAXLINE];
    struct epoll_event ev, events[MAX_EVENTS];

    typedef struct sockaddr SA;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    ret = bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    if (ret == -1)
        err_exit("bind");
    
    ret = listen(listenfd, 5);
    if (ret == -1)
        err_exit("listen");

    epfd = epoll_create1(0);
    if (epfd == -1)
        err_exit("epoll_create1");

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (ret == -1)
        err_exit("epoll_ctl: listenfd");
    
    std::list<std::shared_ptr<Connection> > conn_grp;

    for ( ; ; ) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            err_exit("epoll_wait");

        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                //man 2 accept
                cliaddrlen = sizeof(cliaddr);
                connfd = accept(listenfd, (SA *)&cliaddr, &cliaddrlen);
                if (connfd == -1)
                    err_exit("accept");
                
                setnonblocking(connfd);
                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
                if (ret == -1)
                    err_exit("epoll_ctl: listenfd");
            }
            else {
                connfd = events[i].data.fd;
                conn_grp.push_front(std::make_shared<Connection>(connfd));
                conn_grp.front()->getRequest();

            }
        }


        
        //conn_grp.push_front(std::make_shared<Connection>(connfd));
        //conn_grp[conn_grp.size()-1]->getRequest();
        //Connection conn(connfd);
        //conn.getRequest();
        
        //n = read(connfd, buff_r, MAXLINE);
        //fp = open("index.html", O_RDONLY);
        //read(fp, buff_s, MAXLINE);
        //close(fp);
        //sprintf(buff_s, "hahaha");
        //write(connfd, buff_s, strlen(buff_s));
        //buff_r[n] = '\0';
        //printf("%s", buff_r);

        //close(connfd);
    }
    close(epfd);
    close(listenfd);
    return 0;
}

