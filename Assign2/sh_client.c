#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

const int MAXSIZE = 250, CHUNKSIZE = 50;

int findMin(int a, int b)
{
    return (a < b) ? a : b;
}

int main()
{       
    int sockfd;                                                  // socket descriptor
    struct sockaddr_in servAddr;
    char* buffer = (char*)malloc(MAXSIZE*sizeof(char));          // buffer for taking input from user

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error in creating socket!\n");
        exit(0);
    }   

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);                            // converting to network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(connect(sockfd, (struct sockaddr* )&servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error in connecting!\n");
        exit(0);
    }
    printf("Connected to server...\n");

    int temp;
    temp = recv(sockfd, buffer, sizeof(buffer), 0);
    if(temp <= 0)                                               // if server socket gets closed or some error in recv
    {
        printf("Error in receiving message from server...\n");
        exit(0);
    }

    printf("%s ", buffer);                                      // print "LOGIN:"
    scanf("%s", buffer);                                        // Taking username as input
    getchar();                                                  // to take '\n' which otherwise would affect later
    buffer[strlen(buffer)] = '\0';
    temp = send(sockfd, buffer, strlen(buffer)+1, 0);           // Sending username to server 
    temp = recv(sockfd, buffer, MAXSIZE, 0);                    // Receiving verification message from server

    if(strcmp(buffer, "NOT-FOUND") == 0)  
    {
        printf("Invalid Username!\n");
        exit(0);
    }       

    char* temp_buf; int cnt, totalSent, idx; size_t len;
    while(1)                                                    // Interacting like a shell with user
    {
        len = 0;
        printf("Enter command > ");
        cnt = getline(&temp_buf, &len, stdin);                  // Taking command as input from user
        
        for(int i = 0; i < cnt-1; i++)
        buffer[i] = temp_buf[i];
        buffer[cnt-1] = '\0';
        free(temp_buf);

        for(int i = 0; i < 5; i++)                              // Sending a chunk of 50 bytes in each iteration
        {
            if(i*CHUNKSIZE >= cnt)                              // Checking if we have already sent the whole buffer in prev iteration
            break;  
            totalSent = 0;
            while(1)                                            // Loop for sending expression to server
            {
                temp = send(sockfd, buffer+totalSent+CHUNKSIZE*i, findMin(CHUNKSIZE, cnt-i*CHUNKSIZE)-totalSent, 0);
                totalSent += temp;
                if(totalSent == findMin(CHUNKSIZE, cnt-i*CHUNKSIZE))                       // Keep sending until total no. of char sent become equal to cntChars
                break;
            }
        }  
        
        if(strcmp(buffer, "exit") == 0)
        {
            printf("Hope you have liked the experience.Bye!\n");
            break;
        }
        
        while(1)                                                // For receiving result from server
        {
            temp_buf = (char*)calloc(CHUNKSIZE, sizeof(char));
            cnt = recv(sockfd, temp_buf, CHUNKSIZE, 0);
            
            if(cnt <= 0)                                        // if server socket gets closed or some error in recv
            {
                printf("Error in receiving result from server...\n");
                exit(0);
            }
            
            if(cnt == 5)
            {
                if(temp_buf[0] == '#' && temp_buf[1] == '#' && temp_buf[2] == '#' && temp_buf[3] == '#' && temp_buf[4] == '\0')
                {
                    printf("Error in running command...");
                    break;
                }
                else if(temp_buf[0] == '$' && temp_buf[1] == '$' && temp_buf[2] == '$' && temp_buf[3] == '$' && temp_buf[4] == '\0')
                {
                    printf("Invalid Command!");
                    break;
                }
                else
                printf("%s", temp_buf);
            }
            else                                                // Printing chunk of result as soon as we received it from server
            printf("%s", temp_buf);
            
            if(temp_buf[cnt-1] == '\0')
            break;

            free(temp_buf);
        }
        free(temp_buf);
        printf("\n");
    }
    
    free(buffer);
    close(sockfd);
    return 0;
}
