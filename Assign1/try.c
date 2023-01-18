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
    char *p; size_t len = 0;
    int cnt = getline(&p, &len, stdin);
    printf("%s %d", p, cnt);
    return 0;
}