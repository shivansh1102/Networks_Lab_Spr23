#ifndef __MYSOCKETH
#define __MYSOCKETH

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h> 
#define SOCK_MyTCP 3757
#define MAX_MESSAGE_SIZE 1000
#define MAX_MESSAGE_TABLE_SIZE 10

typedef struct{
    void *buf;
    int len;
    int flags;
}message;

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_listen(int sockfd, int backlog);

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

ssize_t my_send(int sockfd, void *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);

int my_close(int fd);


#endif
