#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int sockfd, newsockfd;
    struct sockaddr_in servAddr, cliAddr;
    socklen_t clilen = 0;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in binding.\n");
        exit(1);
    }
    listen(sockfd, 5);
    int cntRecv, cntSent;
    char buf[100];
    while(1)
    {
        if((newsockfd = accept(sockfd, (struct sockaddr*)&cliAddr, &clilen)) < 0)
        {
            perror("Error in accepting\n");
            exit(1);
        }
        char* cliIP = inet_ntoa(cliAddr.sin_addr);
        if(strncmp(cliIP, "144.25", 6) == 0)
        {
            printf("Not good IP\n");
            close(newsockfd);
            exit(0);
        }
        if(fork() == 0)
        {
            while(1)
            {
                cntRecv = recv(newsockfd, buf, sizeof(buf), 0);
                buf[cntRecv] = '\0';
                printf("Server received : %s\n", buf);
                if(cntRecv == 0)        // client closes the connection
                break;
                cntSent = 0;
                while(1)
                {
                    int temp = send(newsockfd, buf+cntSent, cntRecv-cntSent, 0);
                    cntSent += temp;
                    if(cntSent == cntRecv)
                    break;
                }
            }
        }
        close(newsockfd);
    }
    return 0;
}