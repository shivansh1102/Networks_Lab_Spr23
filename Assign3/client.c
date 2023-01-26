#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    int sockfd;                                                                     // socket descriptor
    char buffer[30];                                                                // to store recieved date and time from server
    struct sockaddr_in servAddr;
    int serverPort, clientPort, temp, cntRecv = 0;

    if(argc > 1)                                                                    // Handling command-line argument
    serverPort = atoi(argv[1]);
    else
    {
        printf("Please enter load balancer's port as command line argument next time.\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(serverPort);
    inet_aton("127.0.0.1", &servAddr.sin_addr);                                     // set the address
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)                              // Creating socket
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    if(connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)        // Connecting to load balancer
    {
        printf("Error in connecting to load balancer\n");
        exit(0);
    }
    printf("Connected to load balancer at port %d...\n", serverPort);

    for(int i = 0; i < 30; i++)
    buffer[i] = '\0';

    while(1)
    {
        int temp = recv(sockfd, buffer+cntRecv, sizeof(buffer)-cntRecv, 0);         // receiving date and time from server in buffer
        cntRecv += temp;
        if(buffer[cntRecv-1] == '\0')
        break;
    }

    printf("Date and time as received from server is as follows :- \n");
    printf("%s\n", buffer);

    close(sockfd);
    return 0;
}