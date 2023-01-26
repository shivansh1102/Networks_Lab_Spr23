#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

int main(int argc, char* argv[])
{
    int sockfd, newsockfd1, newsockfd2;                                          // Socket Descriptors
    char buffer[30];
    struct sockaddr_in servAddr1, servAddr2, cliAddr, ownAddr;
    int serv1Port, serv2Port, ownPort;
    char* IP = "127.0.0.1";

    if(argc > 3)                                                                 // Handling command-line argument
    {            
        ownPort = atoi(argv[1]);             
        serv1Port = atoi(argv[2]);           
        serv2Port = atoi(argv[3]);           
    }            
    else                                                                         // If less than 3 arguments provided by user, then exit
    {
        printf("Please enter load manager's port & both servers' port as command line argument next time.\n");
        exit(0);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    servAddr1.sin_family = AF_INET;                                              // Defining things for 1st server
    servAddr1.sin_port = htons(serv1Port);           
    inet_aton(IP, &servAddr1.sin_addr);

    servAddr2.sin_family = AF_INET;                                              // Defining things for 2nd server
    servAddr2.sin_port = htons(serv2Port);           
    inet_aton(IP, &servAddr2.sin_addr);     

    ownAddr.sin_family = AF_INET;                                                // Defining things for load balancer
    ownAddr.sin_port = htons(ownPort);
    inet_aton(IP, &ownAddr.sin_addr);

    if((bind(sockfd, (struct sockaddr* )&ownAddr, sizeof(ownAddr))) < 0)
    {
        perror("Error in binding sockfd\n");
        exit(0);
    }

    listen(sockfd, 5);

    int ret, timeout = 5000, cntSent, cntRecv, temp, cliLength, loadServ1 = 0, loadServ2 = 0;
    time_t currTime, prevTime; 
    while(1)
    {
        struct pollfd fdset;
        fdset.fd = sockfd;
        fdset.events = POLLIN;

        time(&prevTime);                                                         // Stored the time before entering into poll

        ret = poll(&fdset, 1, timeout);

        if(ret < 0)
        {
            printf("Error in poll!\n");
            exit(0);
        }
        else if(ret == 0)                                                       // Came out of poll due to timeout => ask for load from servers
        {
            if((newsockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)              // Creating newsockfd1 to communicate with server1
            {
                printf("Error in creating socket\n");
                exit(0);
            }
            if((newsockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)              // Creating newsockfd1 to communicate with server1
            {
                printf("Error in creating socket\n");
                exit(0);
            }
            if(connect(newsockfd1, (struct sockaddr* )&servAddr1, sizeof(servAddr1)) < 0)
            {
                printf("Error in connecting with server 1\n");
                exit(0);
            }
            if(connect(newsockfd2, (struct sockaddr* )&servAddr2, sizeof(servAddr2)) < 0)
            {
                printf("Error in connecting with server 2\n");
                exit(0);
            }
            
            strcpy(buffer, "Send Load");
            cntSent = 0;
            while(1)                                                            // Sending to server1
            {
                temp = send(newsockfd1, buffer+cntSent, strlen(buffer) + 1 - cntSent, 0);
                cntSent += temp;
                if(cntSent == strlen(buffer)+1)
                break;
            } 
            cntSent = 0;
            while(1)                                                            // Sending to server2
            {
                temp = send(newsockfd2, buffer+cntSent, strlen(buffer) + 1 - cntSent, 0);
                cntSent += temp;
                if(cntSent == strlen(buffer)+1)
                break;
            } 

            cntRecv = 0;                                                        // receiving load from server 1
            while(1)
            {
                temp = recv(newsockfd1, buffer+cntRecv, sizeof(buffer)-cntRecv, 0);      
                cntRecv += temp;
                if(buffer[cntRecv-1] == '\0')
                break;
            }
            loadServ1 = atoi(buffer);
            printf("Load received from server, IP: %s, Port: %d :- %d\n", IP, serv1Port, loadServ1);
            
            cntRecv = 0;                                                         // receiving load from server 2
            while(1)
            {
                temp = recv(newsockfd2, buffer+cntRecv, sizeof(buffer)-cntRecv, 0);      
                cntRecv += temp;
                if(buffer[cntRecv-1] == '\0')
                break;
            }
            loadServ2 = atoi(buffer);
            printf("Load received from server, IP: %s, Port: %d :- %d\n", IP, serv2Port, loadServ2);

            close(newsockfd1);
            close(newsockfd2);
            timeout = 5000;                                                      // reset timeout to 5sec
        }
        else                                                                     // came out of poll due to some client request
        {
            cliLength = 0;                                                       // newsockfd1 will communicate with client
            if((newsockfd1 = accept(sockfd, (struct sockaddr* )&cliAddr, &cliLength)) < 0)      
            {
                printf("Error in accepting from client\n");
                exit(0);
            }

            if(fork() == 0)                                                      // Child process will communicate with client through newsockfd1
            {
                // sleep(3);                                                     // for testing concurrency
                close(sockfd);

                if((newsockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)           // newsockfd2 will communicate with server
                {
                    printf("Error in creating socket\n");
                    exit(0);
                }
                if(loadServ1 < loadServ2)                                        // Connecting to server with less load
                {
                    if(connect(newsockfd2, (struct sockaddr* )&servAddr1, sizeof(servAddr1)) < 0)
                    {
                        printf("Error in connecting with server 1\n");
                        exit(0);
                    }
                    printf("Sending client request to server, IP: %s, Port: %d ...\n", IP, serv1Port);
                }
                else
                {
                    if(connect(newsockfd2, (struct sockaddr* )&servAddr2, sizeof(servAddr2)) < 0)
                    {
                        printf("Error in connecting with server 2\n");
                        exit(0);
                    }
                    printf("Sending client request to server, IP: %s, Port: %d ...\n", IP, serv2Port);
                }

                strcpy(buffer, "Send Time");
                cntSent = 0;
                while(1)                                                         // sending "Send Time" to server
                {
                    temp = send(newsockfd2, buffer+cntSent, strlen(buffer) + 1 - cntSent, 0);
                    cntSent += temp;
                    if(cntSent == strlen(buffer)+1)
                    break;
                } 

                cntRecv = 0;                                                     // receiving date & time from server
                while(1)
                {
                    temp = recv(newsockfd2, buffer+cntRecv, sizeof(buffer)-cntRecv, 0);      
                    cntRecv += temp;
                    if(buffer[cntRecv-1] == '\0')
                    break;
                }
                close(newsockfd2);

                cntSent = 0;
                while(1)                                                         // sending date & time to client
                {
                    temp = send(newsockfd1, buffer+cntSent, strlen(buffer) + 1 - cntSent, 0);
                    cntSent += temp;
                    if(cntSent == strlen(buffer)+1)
                    break;
                } 
                close(newsockfd1);
                exit(0);
            }

            close(newsockfd1);
            time(&currTime);

            timeout -= difftime(currTime, prevTime)*1000;                        // updating timeout
        }
    }

    close(sockfd);
    return 0;       
}