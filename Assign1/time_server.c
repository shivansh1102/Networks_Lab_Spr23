#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main()
{
    int sockfd, newsockfd;                           // socket descriptors
    char buffer[100];                                // To store current system date and time
    struct sockaddr_in servAddr, cliAddr;            // structure describing internet socket address

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Creating sockfd
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);                // changing it to network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);      // set the address

    if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
    {
        printf("Error in binding sockfd\n");
        exit(0);
    }

    listen(sockfd, 5);                               // Atmost 5 clients can be queued while server is running

    int cliLength, temp;
    while(1)
    {
        cliLength = sizeof(cliAddr);
        if((newsockfd = accept(sockfd, (struct sockaddr *) &cliAddr, &cliLength)) < 0)
        {
            printf("Error in accepting\n");
            exit(0);
        }

        time_t timeVar = time(NULL);
		struct tm* local = localtime(&timeVar);     // Getting local time & data from system

		strcpy(buffer, asctime(local));             // copying current date and time in buffer

        // Keep sending until whole string is transferred
        int cntSent = 0;
		while(1)
        {
            temp = send(newsockfd, buffer+cntSent, strlen(buffer) + 1 - cntSent, 0);
            cntSent += temp;
            if(cntSent == strlen(buffer)+1)
            break;
        }

        close(newsockfd);  
    }

    close(sockfd);
    return 0;
}
