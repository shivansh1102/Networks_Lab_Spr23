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
    int sockfd;                                         // socket descriptor
    char buffer[30];                                    // to store recieved date and time from server
    struct sockaddr_in servAddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  // Creating socket
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);                   // convert it into network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);         // set the address

    if(connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)    // Connecting to server
    {
        printf("Error in connecting to server\n");
        exit(0);
    }

    for(int i = 0; i < 30; i++)
    buffer[i] = '\0';

    int temp = recv(sockfd, buffer, sizeof(buffer), 0); // receiving date and time from server in buffer

    printf("Date and time as received from server is as follows :- \n");
    printf("%s\n", buffer);

    close(sockfd);
    return 0;
}