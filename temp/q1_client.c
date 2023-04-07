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
    int sockfd;
    struct sockaddr_in servAddr;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in connecting to server\n");
        exit(1);
    }

    char buf[100];
    int cntSent;
    while(1)
    {
        printf("Enter input : ");
        scanf("%s", buf);
        cntSent = 0;
        while(1)
        {
            int temp = send(sockfd, buf+cntSent, strlen(buf)+1-cntSent, 0);
            cntSent += temp;
            if(cntSent == strlen(buf)+1)
            break;
        }
        recv(sockfd, buf, sizeof(buf), 0);
        printf("Message from server : %s\n", buf);
    }
    close(sockfd);
    return 0;
}