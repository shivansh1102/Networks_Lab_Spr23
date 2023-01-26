#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char* argv[])
{
    srand(time(NULL));                                                                  // seeding random number generator with current time

    int sockfd, newsockfd;                                                              // socket descriptors
    char buffer[30];                                                                    
    struct sockaddr_in servAddr, cliAddr;                                               // structure describing internet socket address
    int serverPort;

    if(argc > 1)                                                                        // Handling command-line argument
    serverPort = atoi(argv[1]);
    else
    {
        printf("Please enter server's port as command line argument next time.\n");
        exit(0);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)                                  // Creating sockfd
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(serverPort);                                              // changing it to network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);                                         // set the address

    if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) 
    {
        printf("Error in binding sockfd\n");
        exit(0);
    }

    listen(sockfd, 5);                                                                  

    int cliLength, temp, cntSent, cntRecv;
    while(1)
    {
        cliLength = sizeof(cliAddr);
        if((newsockfd = accept(sockfd, (struct sockaddr *) &cliAddr, &cliLength)) < 0)  // Creating newsockfd to communicate with load balancer
        {
            printf("Error in accepting\n");
            exit(0);
        }

        cntRecv = 0;                                                                    // receiving message from load balancer
        while(1)
        {
            int temp = recv(newsockfd, buffer+cntRecv, sizeof(buffer)-cntRecv, 0);      
            cntRecv += temp;
            if(buffer[cntRecv-1] == '\0')
            break;
        }
        
        if(strcmp(buffer, "Send Load") == 0)
        {
            temp = rand()%100+1;                                                        // Generating random number between 1 and 100
            sprintf(buffer, "%d", temp);
            buffer[strlen(buffer)] = '\0';
            printf("Load Sent: %s\n", buffer);
        }
        else if(strcmp(buffer, "Send Time") == 0)
        {
            time_t timeVar = time(NULL);
            struct tm* local = localtime(&timeVar);                                     // Getting local time & data from system
            strcpy(buffer, asctime(local));                                             // copying current date and time in buffer
            printf("Date & Time Sent: %s\n", buffer);
        }
        else
        {
            strcpy(buffer, "Invalid request");
            printf("Invalid request.\n");
        }

        cntSent = 0;
		while(1)                                                                        // Keep sending until whole string is transferred
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