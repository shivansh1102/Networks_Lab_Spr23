#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

int main(int argc, char* argv[])
{
    int type = 1;
    char filename[100]; int x, y;
    if(argc > 4)
    {
        type = 2;
        strcpy(filename, argv[2]);
        x = atoi(argv[3]);
        y = atoi(argv[4]);
    }
    else
    {
        strcpy(filename, argv[2]);
    }
    
    int sockfd;
    struct sockaddr_in servAddr;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20002);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in connecting to server\n");
        exit(1);
    }

    int cntSent = 0;
    while(cntSent < strlen(argv[1])+1)
    {
        int temp = send(sockfd, argv[1]+cntSent, strlen(argv[1])+1-cntSent, 0);
        cntSent += temp;
    }
    cntSent = 0;
    while(cntSent < strlen(argv[2])+1)
    {
        int temp = send(sockfd, argv[2]+cntSent, strlen(argv[2])+1-cntSent, 0);
        cntSent += temp;
    }

    int cntRecv = 0;
    char buf[100]; 
    if(argc == 3)
    {
        cntRecv = 0;
        while(1)
        {
            int temp = recv(sockfd, buf+cntRecv, sizeof(buf)-cntRecv, 0);
            if(temp == 0)
            break;
            printf("%d\n", temp);
            cntRecv += temp;
        }
        if(cntRecv == 0)
        printf("Error in Deleting...\n");
        else
        printf("%s\n", buf);
    }
    else
    {
        cntSent = 0;
        while(cntSent < strlen(argv[3])+1)
        {
            int temp = send(sockfd, argv[3]+cntSent, strlen(argv[3])+1-cntSent, 0);
            cntSent += temp;
        }
        cntSent = 0;
        while(cntSent < strlen(argv[4])+1)
        {
            int temp = send(sockfd, argv[4]+cntSent, strlen(argv[4])+1-cntSent, 0);
            cntSent += temp;
        }

        cntRecv = 0;
        while(1)
        {
            for(int i = 0; i < 100; i++)
            buf[i] = '\0';
            int temp = recv(sockfd, buf, 99, 0);
            if(temp == 0)
            break;
            cntRecv += temp;
            printf("%s", buf);
        }
        if(cntRecv == 0)
        printf("Error in getting bytes...\n");
        else
        printf("\nGetting bytes successful!\n");
    }
    close(sockfd);
    return 0;
}