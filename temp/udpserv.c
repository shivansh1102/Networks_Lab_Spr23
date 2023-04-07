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
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }
    memset(&servAddr, 0, sizeof(servAddr)); 
    memset(&cliAddr, 0, sizeof(cliAddr)); 
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);
    if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in binding.\n");
        exit(1);
    }
    // while (1)
    // {    
    // // listen(sockfd, 5);
        char buf[100];
        socklen_t clilen;
        
        recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cliAddr, &clilen);
        printf("Received server : %s\n", buf);
        for(int i = 0; i < 5; i++)
        {
            sleep(1);
            sendto(sockfd, buf, 1, 0, (struct sockaddr*)&cliAddr, clilen);
            printf("Message sent\n");
        }
    // }

    close(sockfd);
    return 0;
}