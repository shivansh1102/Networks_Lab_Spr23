#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "poll.h"

int main()
{
    int sockfd;                                         // socket descriptor
    char buffer[30];                                    // to store recieved date and time from server
    struct sockaddr_in servAddr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)   // Creating socket
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    memset(&servAddr, 0, sizeof(servAddr)); 

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);         // set the address


    int ret, cnt;
    char timeExceeded = 't';                            // flag to check whether there is time-out or not
    socklen_t servLength;
    for(int i = 0; i < 5; i++)
    {
        struct pollfd fdset;                            // to handle time-out using polling
        fdset.fd = sockfd;
        fdset.events = POLLIN;

        // Making a send request first so that server can get to know the address of client
        strcpy(buffer, "Please provide me date & time");
        sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr* )&servAddr, sizeof(servAddr));

        ret = poll(&fdset, 1, 3000);                    // Timeout of 3000ms
        if(ret < 0)                                     // Error
        {
            printf("Error in receiving from server!\n");
            exit(0);
        }
        if(ret > 0)                                     // if some event occurs         
        {
            printf("Information received in %d iteration\n", i+1);
            timeExceeded = 'f';
            servLength = sizeof(servAddr);
            cnt = recvfrom(sockfd, buffer, 30, 0, (struct sockaddr*) &servAddr, &servLength);
            buffer[cnt] = '\0';
            break;
        }
    }

    close(sockfd);

    if(timeExceeded == 't')
    printf("Timeout Exceeded!\n");
    else
    printf("Information received from server : %s\n", buffer); 

    return 0;
}