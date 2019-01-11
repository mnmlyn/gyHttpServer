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

#define SERV_PORT 80
#define MAXLINE 4096

void
err_exit(const char *err) {
    fprintf(stderr, "Err:%s\n", err);
    exit(1);
}

int
main(int argc, char **argv) {
    int listenfd, connfd, ret;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddrlen;
    char buff[MAXLINE];

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
    
    ret = listen(listenfd, 100);
    if (ret == -1)
        err_exit("listen");

    for( ; ; ) {
        //man 2 accept
        cliaddrlen = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *)&cliaddr, &cliaddrlen);
        if (connfd == -1)
            err_exit("accept");

        sprintf(buff, "hahaha");
        write(connfd, buff, strlen(buff));
        read(connfd, buff, MAXLINE);
        printf("%s", buff);

        close(connfd);
    }
    close(listenfd);
    return 0;
}



