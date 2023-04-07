#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

int main()
{
    int sockfd;
    struct sockaddr_in servAddr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr)); 

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    char buf[100];
    strcpy(buf, "helloo");
    socklen_t servlen = sizeof(servAddr);
    sendto(sockfd, buf, strlen(buf)+1, 0, (struct sockaddr*)&servAddr, servlen);
    

    struct pollfd fdset;
    fdset.fd = sockfd;
    fdset.events = POLLIN;
    int ret = poll(&fdset, 1, 10000);
    if(ret < 0)
    {
        perror("Error in poll\n");
        exit(1);
    }
    if(ret == 0)
    printf("Time out\n");
    if(recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&servAddr, &servlen)<0){
        perror("wqwe");
    }
    printf("Client received: %s", buf);
    return 0;
}