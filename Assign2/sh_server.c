#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

const int MAXSIZE = 250, CHUNKSIZE = 50;

char* handle_pwd()
{
    return "pwd";
}

char* handle_dir(char* argument)
{
    return "dir";
}

char* handle_cd(char* argument)
{
    return "cd";
}

int main()
{
    int sockfd, newsockfd;                                                  // socket descriptors
    struct sockaddr_in servAddr, cliAddr;
    char* buffer = (char*)malloc(MAXSIZE*sizeof(char));                     // buffer for taking input from user
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error in creating socket!\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);                                       // changing it to network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);
    
    if(bind(sockfd, (struct sockaddr* )&servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error in binding!\n");
        exit(0);
    }

    listen(sockfd, 5);                                                      // Atmost 5 clients can be queued while server is running

    int cliLength, temp, idx, cnt; char validUser;
    char* temp_buf = (char*)malloc(CHUNKSIZE * sizeof(char));
    char* result;
    while(1)
    {
        if((newsockfd = accept(sockfd, (struct sockaddr* )&cliAddr, &cliLength)) < 0)
        {
            printf("Error in accepting\n");
            exit(0);
        }

        if(fork() == 0)                                                     // Child process will interact with client & parent process is still there to accept from other clients 
        {
            strcpy(buffer, "LOGIN:");
            temp = send(newsockfd, buffer, strlen(buffer)+1, 0);            // Sending Login message
            temp = recv(newsockfd, buffer, MAXSIZE, 0);                     // Receiving username from client
            printf("Username received from client : %s\n", buffer);

            validUser = 'f'; 
            FILE *fileptr = fopen("users.txt", "r");
            if(fileptr == NULL)
            {
                printf("Error in opening users.txt\n");
                exit(0);
            }

            while(fgets(temp_buf, MAXSIZE, fileptr) != NULL)                // Checking whether username is valid or not
            {
                temp_buf[strcspn(temp_buf, " \t\n")] = '\0';
                // printf("%s %d\n", temp_buf, (int)strlen(temp_buf));
                printf("%d %s %s\n", strcmp(temp_buf, buffer), temp_buf, buffer);
                if(strcmp(temp_buf, buffer) == 0)
                {
                    validUser = 't';
                    break;
                }
            }
            fclose(fileptr);
            if(validUser == 'f')
            {
                strcpy(buffer, "NOT-FOUND");
                temp = send(newsockfd, buffer, strlen(buffer)+1, 0);
                exit(0);
            }
            strcpy(buffer, "FOUND");
            temp = send(newsockfd, buffer, strlen(buffer)+1, 0);            

            while(1)
            {
                idx = 0;
                while(1)                                                        // For receiving command from client
                {
                    cnt = recv(newsockfd, temp_buf, CHUNKSIZE, 0);
                    printf("%d\n", cnt);
                    if(cnt <= 0)                                                // if client socket gets closed or some error in recv
                    {
                        printf("Error in receiving command from client...\n");
                        exit(0);
                    }

                    for(int i = 0; i < cnt; i++)
                    buffer[idx++] = temp_buf[i];

                    if(buffer[idx-1] == '\0')
                    break;
                }
                printf("Command received from client : %s %d\n", buffer, (int)strlen(buffer));

                if(strcmp(buffer, "exit") == 0)
                {
                    printf("Bye!\n");
                    break;
                }
                else if(strlen(buffer) == 3 && buffer[0] == 'p' && buffer[1] == 'w' && buffer[2] == 'd')
                result = handle_pwd();
                else if(strlen(buffer) >= 3 && buffer[0] == 'd' && buffer[1] == 'i' && buffer[2] == 'r')
                result = handle_dir(buffer+3);
                else if(strlen(buffer) >= 2 && buffer[0] == 'c' && buffer[1] == 'd')
                result = handle_cd(buffer+2);
                else
                result = "$$$$";

                send(newsockfd, result, strlen(result)+1, 0);
            }
            exit(0);
        }
        
        close(newsockfd);
    }   

    close(sockfd);
    return 0;
}