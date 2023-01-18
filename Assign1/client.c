/*

    ASSUMPTION : Total length of expression is not too large 
                 that it cannot be stored as a whole in memory.

*/
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
    int sockfd;                                                                 // socket descriptor
    char* expression, *temp;                                                    // to store expression provided by user
    struct sockaddr_in servAddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)                          // Creating socket
    {
        printf("Erorr in creating socket\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);                                           // convert it into network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);                                 // set the address

    if(connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)    // Connecting to server
    {
        printf("Error in connecting to server\n");
        exit(0);
    }

    int cntChars, totalSent; 
    size_t len; 
    char buffer[30];                                                            // to store the answer received from server

    while(1)
    {
        len = 0;
        printf("Enter the expression you want to be evaluated or enter -1 to exit : \n");
        cntChars = getline(&temp, &len, stdin);                                 // reading expression from user

        expression = (char*)malloc(cntChars * sizeof(char));                    
        for(int i = 0; i < cntChars-1; i++)                                     // Copying in expression
        expression[i] = temp[i];
        expression[cntChars-1] = '\0';

        totalSent = 0;
        while(1)                                                                // Loop for sending expression to server
        {
            int temp2 = send(sockfd, expression+totalSent, cntChars-totalSent, 0);
            totalSent += temp2;
            if(totalSent == cntChars)                                           // Keep sending until total no. of char sent become equal to cntChars
            break;
        }
        
        if(strcmp(expression, "-1") == 0)
        {
            printf("Bye!\n");
            break;
        }
        
        recv(sockfd, buffer, sizeof(buffer), 0);                                // Receiving result from server, assuming it is small enough to be received in one go
        printf("Final result received from server : %s\n", buffer);

        free(temp);
        free(expression);
    }

    close(sockfd);
    return 0;
}

// Test case : (3.5+6.3)*(2.36+9.34*8)/(3.2-5.6+9.87)