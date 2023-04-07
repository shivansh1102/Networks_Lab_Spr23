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
    struct sockaddr_in servAddr, cliAddr;   
    socklen_t clilen;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20002);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in binding.\n");
        exit(1);
    }
    listen(sockfd, 5);

    int S[50]; struct pollfd fdset[51];
    for(int i = 0; i < 50; i++)
    S[i] = -1;
    int ret; char buf[1000];
    while(1)
    {
        for(int i = 0; i < 50; i++)
        {
            fdset[i].fd = S[i];
            fdset[i].events = POLLIN;
        }
        fdset[50].fd = sockfd;
        fdset[50].events = POLLIN;
        ret = poll(fdset, 51, -1);
        if(ret < 0)
        {
            perror("Error in polling\n");
            exit(1);
        }
        if(ret == 0)
        continue;
        for(int i = 50; i >= 0; i--)
        {
            if(fdset[i].revents & POLLIN)
            {
                printf("hello %d\n", i);
                if(i == 50) // new connection
                {
                    int j = 0;
                    while(j < 50 && S[j] != -1)
                    j++;
                    // printf("j=%d\n", j);
                    if((S[j] = accept(sockfd, (struct sockaddr*)&cliAddr, &clilen)) < 0)
                    {
                        perror("Error in accepting\n");
                        exit(1);
                    }
                    printf("Connection with client in position %d established\n", j);
                    fdset[j].fd = S[j];
                }
                else
                {
                    int temp = recv(S[i], buf, sizeof(buf), 0);
                    buf[temp] = '\0';
                    printf("%d bytes received : %s from client %d\n", temp, buf, i);
                    close(S[i]);
                    S[i] = -1;
                    fdset[i].fd = -1;
                }
                // break;
            }
        }
    }
    close(sockfd);
    return 0;
}