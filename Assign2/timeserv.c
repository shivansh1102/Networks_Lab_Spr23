#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>


int main()
{
    int sockfd;                                             // socket descriptor
    char buffer[30];                                        // To store current system date and time
    struct sockaddr_in servAddr, cliAddr;                   // structure describing internet socket address

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)       // Creating socket
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    memset(&servAddr, 0, sizeof(servAddr)); 
    memset(&cliAddr, 0, sizeof(cliAddr)); 

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);
    inet_aton("127.0.0.1", &servAddr.sin_addr);              // set the address

    if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
    {
        printf("Error in binding sockfd\n");
        exit(0);
    }

    printf("Server started on port 20000\n");

    socklen_t cliLength; int cnt;
    while(1)
    {
        cliLength = sizeof(cliAddr);
        // First making a recv call to know the address of client, after which it can make a send call
        cnt = recvfrom(sockfd, buffer, 30, 0, (struct sockaddr*) &cliAddr, &cliLength);
        buffer[cnt] = '\0';
        printf("Message received from client : %s\n", buffer);

        time_t timeVar = time(NULL);
		struct tm* local = localtime(&timeVar);             // Getting local time & data from system
        // sleep(10);                                        // For testing
		strcpy(buffer, asctime(local));                     // copying current date and time in buffer

        sendto(sockfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*) &cliAddr, cliLength);
        printf("Date and time information sent from server.\n");
    }

    close(sockfd);

    return 0;
}