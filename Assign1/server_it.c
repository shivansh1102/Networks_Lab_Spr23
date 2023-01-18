#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

char divideByZero = 'f', invalidExpr = 'f';
// This function solves the subexpression [i...j] where j is the matching bracket of expression[i].
// It is almost same as evaluateExpression(), just a few changes.
double solveWithinBracket(char* expression, int len, int *i_ptr)
{
    int i = *i_ptr;
    i++;
    double ans = 0, curr, multiplier;
    char operator = '\0';
    for(; expression[i] != ')'; i++)
    {
        if(expression[i] == ' ')
        continue;
        if(expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/')
        operator = expression[i];
        else
        {
            if(expression[i] == '(')
            {
                curr = solveWithinBracket(expression, len, &i);
                if(divideByZero == 't' || invalidExpr == 't') // If error occurs in recursive solveWithinBracket(), no need to execute further 
                return 0;
            }
            else                                        // Extracting current number
            {
                curr = 0;
                while(expression[i] >= '0' && expression[i] <= '9')
                {
                    curr = curr*10 + (expression[i]-'0');
                    i++;
                }
                if(expression[i] == '.')
                i++;
                multiplier = 0.1;
                while(expression[i] >= '0' && expression[i] <= '9')
                {
                    curr = curr + multiplier*(expression[i]-'0');
                    multiplier /= 10;
                    i++;
                }
                i--;                                    // So that it points to next character after getting incremented in for loop
            }
            
            switch(operator)                            // Performing operation as specified by "operator"
            {
                case '\0' :                             // Leftmost number after '('
                ans = curr;
                break;
                case '+' :
                ans += curr;
                break;
                case '-' :
                ans -= curr;
                break;
                case '*' :
                ans *= curr;
                break;
                case '/' :
                if(abs(curr) < 1e-9)                    // Handling divide by 0
                {
                    divideByZero = 't';
                    printf("Divide by zero!\n");
                    return 0;
                }
                ans /= curr;
                break;
                default : 
                printf("Invalid operator\n");
                invalidExpr = 't';
                return 0;
            }
        }
    }
    *i_ptr = i;                                        // Updating i_ptr
    return ans;
}

double evaluateExpression(char* expression, int len)
{
    double ans = 0, curr, multiplier;
    char operator = '\0';                              // stores current operator which needs to be applied
    for(int i = 0; i < len; i++)
    {
        if(expression[i] == ' ')
        continue;
        if(expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/')
        operator = expression[i];
        else
        {
            if(expression[i] == '(')
            {
                curr = solveWithinBracket(expression, len, &i);
                if(divideByZero == 't' || invalidExpr == 't') // If error occurs in solveWithinBracket(), no need to execute further 
                return 0;
            }
            else                                      // Extracting current number
            {
                curr = 0;
                while(expression[i] >= '0' && expression[i] <= '9')
                {
                    curr = curr*10 + (expression[i]-'0');
                    i++;
                }
                if(expression[i] == '.')
                i++;
                multiplier = 0.1;
                while(expression[i] >= '0' && expression[i] <= '9')
                {
                    curr = curr + multiplier*(expression[i]-'0');
                    multiplier /= 10;
                    i++;
                }
                i--;                                 // So that it points to next character after getting incremented in for loop
            }
            
            switch(operator)                         // Performing operation as specified by "operator"
            {
                case '\0' : 
                ans = curr;
                break;
                case '+' :
                ans += curr;
                break;
                case '-' :
                ans -= curr;
                break;
                case '*' :
                ans *= curr;
                break;
                case '/' :
                if(abs(curr) < 1e-9)                // Handling divide by 0
                {
                    divideByZero = 't';
                    printf("Divide by zero!\n");
                    return 0;
                }
                ans /= curr;
                break;
                default : 
                printf("Invalid operator\n");
                invalidExpr = 't';
                return 0;
            }
        }
    }
    return ans;
}


int main()
{
    int sockfd, newsockfd;                          // Socket descriptors
    struct sockaddr_in servAddr, cliAddr;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error in creating socket\n");
        exit(0);
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(20000);               // changing it to network byte order
    inet_aton("127.0.0.1", &servAddr.sin_addr);     // set the address

    if(bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error in binding\n");
        exit(0);
    }

    listen(sockfd, 5);                              // Atmost 5 clients can be queued while server is running

    int cliLength, exprSize;
    char *expression, *currChunk;
    const int CHUNK_SIZE = 10;
    double ans; 
    char* ans_str = (char*) malloc(30 * sizeof(char));

    while(1)
    {
        cliLength = sizeof(cliAddr);
        if((newsockfd = accept(sockfd, (struct sockaddr *) &cliAddr, &cliLength)) < 0)
        {
            printf("Error in accepting\n");
            exit(0);
        }
        
        while(1)                                                                // Dealing with a client through newsockfd
        {
            expression = (char*)malloc(sizeof(char) * CHUNK_SIZE);              // "expression" will store whole expression provided by client
            exprSize = 0;                                                       // "exprSize" will store total count of bytes received from client
            while(1)                                                            // Receiving expression from client
            {
                currChunk = (char*)malloc(sizeof(char) * CHUNK_SIZE);           // To store current chunk received from client
                int cnt = recv(newsockfd, currChunk, CHUNK_SIZE, 0);
                
                if(cnt == 0)                                                    // client server might get closed
                break;

                for(int i = 0; i < cnt; i++)                                    // Copying in expression
                {
                    expression[exprSize++] = currChunk[i];
                }

                free(currChunk);
                if(expression[exprSize-1] == '\0')                              // Reached the end of entire expression
                break;

                expression = (char*)realloc(expression, sizeof(char) * (exprSize + CHUNK_SIZE));
            }

            if(strcmp(expression, "-1") == 0 || exprSize == 0)                  // Terminate on receiving "-1"
            {
                printf("Hope you have liked the experience. Bye!\n");
                break;
            }

            printf("Received %d bytes from client\n", exprSize);
            printf("Given expression : %s\n", expression);

            ans = evaluateExpression(expression, exprSize-1);
            if(divideByZero == 'f' && invalidExpr == 'f')
            {
                printf("Answer is %lf\n", ans);
                sprintf(ans_str, "%lf", ans);
            }
            else if(divideByZero = 't')
            {
                strcpy(ans_str, "Divide by zero!");
                divideByZero = 'f';
            }
            else
            {
                strcpy(ans_str, "Invalid Expression!");
                invalidExpr = 'f';
            }
            send(newsockfd, ans_str, strlen(ans_str)+1, 0);

            free(expression);
        }

        close(newsockfd);
    }

    free(ans_str);
    close(sockfd);
    return 0;
}