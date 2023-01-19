#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <dirent.h>

const int MAXSIZE = 250, CHUNKSIZE = 50;                                                               
                                                            
int findMin(int a, int b)
{
    return (a < b) ? a : b;
}

char* handle_pwd(char* argument)
{
    char* res = (char*)malloc(sizeof(char) * MAXSIZE);
    while(argument[0] == ' ' || argument[0] == '\t')
    argument++;

    if(argument[0] != '\0')                                                 // Invalid command
    strcpy(res, "$$$$");
    else if(getcwd(res, MAXSIZE) == NULL)                                   // In case of error
    strcpy(res, "####");

    return res;
}

void handle_dir(int newsockfd, char* argument)
{
    char *temp_buf = (char*)malloc(sizeof(char) * MAXSIZE);

    if(argument[0] != ' ' && argument[0] != '\t' && argument[0] != '\0')    // Invalid command with dir as prefix
    {
        strcpy(temp_buf, "$$$$");
        send(newsockfd, temp_buf, 5, 0);
        return;
    }

    while(argument[0] == ' ' || argument[0] == '\t')
    argument++;

    if(argument[0] == '\0')                                                 // If there is no argument, then dir for current directory
    {
        argument[0] = '.';
        argument[1] = '\0';
    }

    struct dirent* dirEntry;                                                // Directory entry pointer
    DIR * dr = opendir(argument);
    if(dr == NULL)                                                          // In case of error
    {
        strcpy(temp_buf, "####");
        send(newsockfd, temp_buf, 5, 0);
        return;
    }
    
    int cnt;
    while((dirEntry = readdir(dr)) != NULL)                                 // Iterating through all
    {
        temp_buf = (char*)calloc(MAXSIZE, sizeof(char));
        strcpy(temp_buf, dirEntry->d_name);
        cnt = strlen(temp_buf)+1; 

        temp_buf[cnt-1] = '\n';

        for(int i = 0; i < 5; i++)                                          // Sending a chunk of 50 bytes in each iteration
        {
            if(i*CHUNKSIZE >= cnt)                                          // Checking if we have already sent the whole buffer in prev iteration
            break;  
            send(newsockfd, temp_buf, findMin(cnt-i*CHUNKSIZE, CHUNKSIZE), 0);
        }  
        free(temp_buf);
    }
    char str[1];                                                            // To denote end of result send to client side
    str[0] = '\0';
    send(newsockfd, str, 1, 0);
}

char* handle_cd(char* argument)
{
    if(argument[0] != ' ' && argument[0] != '\t' && argument[0] != '\0')    // Invalid command with cd as prefix
    return "$$$$";

    while(argument[0] == ' ' || argument[0] == '\t')
    argument++;

    if(argument[0] == '\0')                                                 // If there is no argument, then cd to home directory
    {
        struct passwd *pword = getpwuid(getuid());
        const char* homeDirectory = pword->pw_dir;                          // Finding home directory

        if(chdir(homeDirectory) < 0)                                        // In case of error
        return "####";

        return "\0";
    }

    if(chdir(argument) < 0)                                                 // In case of error
    return "####";

    return "\0";
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

    int cliLength, temp, idx, cnt, totalSent; char validUser;
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
                temp_buf[strcspn(temp_buf, " \t\n")] = '\0';                // Removing trailing space and tab
                if(strcmp(temp_buf, buffer) == 0)
                {
                    validUser = 't';
                    break;
                }
            }
            fclose(fileptr);

            if(validUser == 'f')
            {
                printf("Invalid User\n");
                strcpy(buffer, "NOT-FOUND");
                temp = send(newsockfd, buffer, strlen(buffer)+1, 0);
                exit(0);
            }
            printf("Valid User\n");
            strcpy(buffer, "FOUND");
            temp = send(newsockfd, buffer, strlen(buffer)+1, 0);            

            while(1)
            {
                idx = 0;
                while(1)                                                        // For receiving command from client
                {
                    cnt = recv(newsockfd, temp_buf, CHUNKSIZE, 0);
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
                printf("Command received from client : %s\n", buffer);

                if(strcmp(buffer, "exit") == 0)
                {
                    printf("Bye!\n");
                    break;
                }
                else if(strlen(buffer) >= 3 && buffer[0] == 'p' && buffer[1] == 'w' && buffer[2] == 'd')
                result = handle_pwd(buffer+3);
                else if(strlen(buffer) >= 3 && buffer[0] == 'd' && buffer[1] == 'i' && buffer[2] == 'r')
                {
                    handle_dir(newsockfd, buffer+3);
                    continue;
                }
                else if(strlen(buffer) >= 2 && buffer[0] == 'c' && buffer[1] == 'd')
                result = handle_cd(buffer+2);
                else
                result = "$$$$";

                cnt = strlen(result)+1;
                for(int i = 0; i < 5; i++)                                      // Sending a chunk of 50 bytes in each iteration
                {
                    if(i*CHUNKSIZE >= cnt)                                      // Checking if we have already sent the whole buffer in prev iteration
                    break;  
                    totalSent = 0;
                    while(1)                                                    // Loop for sending expression to server
                    {
                        temp = send(newsockfd, result+totalSent+CHUNKSIZE*i, findMin(CHUNKSIZE, cnt-i*CHUNKSIZE)-totalSent, 0);
                        totalSent += temp;
                        if(totalSent == findMin(CHUNKSIZE, cnt-i*CHUNKSIZE))                       // Keep sending until total no. of char sent become equal to cntChars
                        break;
                    }
                }  
            }
            exit(0);
        }
        
        close(newsockfd);
    }   

    close(sockfd);
    return 0;
}