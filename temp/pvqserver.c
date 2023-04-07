#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <pthread.h>

void* runner(void* param)
{
    printf("Server connected...\n");
    int newsockfd = *((int*)param);
    printf("newsockfd=%d\n", newsockfd);
    char type[10], filename[100], buf[100];
    int cntRecv = 0, cntSent, x, y;
    while(1)
    {
        int temp;
        if((temp = recv(newsockfd, buf+cntRecv, 100-cntRecv, 0)) < 0)
        {
            perror("...");
        }
        cntRecv += temp;
        printf("%d\n", temp);
        if(buf[cntRecv-1] == '\0')
        break;
    }
    for(int i = 0; i < cntRecv; i++)
    {
        type[i]=buf[i];
        if(buf[i] == '\0')
        {
            for(int j = i+1; j < cntRecv; j++)
            filename[j-i-1] = buf[i];
            break;
        }
    }
    printf("type=%s, filename=%s", type, filename);
    // cntRecv = 0;
    // while(1)
    // {
    //     int temp = recv(newsockfd, filename+cntRecv, 100-cntRecv, 0);
    //     cntRecv += temp;
    //     if(filename[cntRecv] == '\0')
    //     break;
    // }
    // printf("type=%s, filename=%s", type, filename);
    if(strcmp(type, "del") == 0)
    {
        printf("hi\n");
        if(remove(filename) == 0)
        {
            strcpy(buf, "delete success");
            cntSent = 0;
            while(cntSent < strlen(buf)+1)
            {
                int temp = send(newsockfd, buf+cntSent, strlen(buf)+1-cntSent, 0);
                cntSent += temp;
            }
            printf("Deleted file %s.\n", filename);
        }
        else
        close(newsockfd);
    }
    else
    {
        cntRecv = 0;
        while(1)
        {
            int temp = recv(newsockfd, buf+cntRecv, 100-cntRecv, 0);
            cntRecv += temp;
            if(buf[cntRecv] == '\0')
            break;
        }
        x = atoi(buf);
        cntRecv = 0;
        while(1)
        {
            int temp = recv(newsockfd, buf+cntRecv, 100-cntRecv, 0);
            cntRecv += temp;
            if(buf[cntRecv] == '\0')
            break;
        }
        y = atoi(buf);

        FILE* fp;
        if((fp = fopen(filename, "r")) == NULL)
        close(newsockfd);
        else
        {
            for(int byteNo = x; byteNo <= y; byteNo++)
            {
                if(fseek(fp, byteNo, SEEK_SET) < 0)
                {
                    close(newsockfd);
                    break;
                }
                buf[0] = fgetc(fp);
                send(newsockfd, buf, 1, 0);
            }
            printf("byte %d to byte %d of file %s sent.\n", x, y, filename);
        }
    }
    pthread_exit(0);
}

int main(int argc, char* argv[])
{
    int sockfd, newsockfd;
    struct sockaddr_in servAddr, cliAddr;   
    socklen_t clilen;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in creating socket\n");
        exit(1);
    }
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20002);
    inet_aton("127.0.0.1", &servAddr.sin_addr);

    if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Error in binding.\n");
        exit(1);
    }
    listen(sockfd, 5);
    while(1)
    {
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if((newsockfd = accept(sockfd, (struct sockaddr*)&cliAddr, &clilen)) < 0)
        {
            perror("Error in accepting...\n");
            exit(1);
        }
        printf("newsockfd=%d\n", newsockfd);
        pthread_create(&tid, &attr, runner, (void*)&newsockfd);
        // close(newsockfd);
    }

    return 0;
}