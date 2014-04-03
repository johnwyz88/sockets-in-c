#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

int listenfd = 0, connfd = 0;
int n;
socklen_t recv_len, cli_len;

typedef struct node
{
    int connfd;
    struct node * next;
} node_t;

node_t * head = NULL;

/* sockaddr_in is a struct from <netinet/in.h> and it has four mem val
 * struct sockaddr_in {
 *    short          sin_family       e.g. AF_INET
 *    unsigned short sin_port;        e.g. htons(3490)
 *    struct in_addr sin_addr;        see struct in_addr, below
 *    char           sin_zero[8];     zero this if prefered
 * };
 *
 * struct in_addr has one unsigned long mem val and it is the most popular 
 * implementation
 * struct in_addr {
 *     unsigned long s_addr;           load with inet_aton() for IPv4 or 
 *                                     inet_pton() for IPv6
 *                                     which convert struct in_addr to 
 *                                     a string in dots and numbers
 *                                     format (e.g. 192.168.5.10) and back
 * };    
 */

struct sockaddr_in serv_addr, cli_addr;

char sendBuff[1025];
char wrtBuff[4096];
char recvBuff[4096];
time_t ticks;
pthread_t pthr;

void * readWrite(void * arg) 
{
    while(1)
    {
        // Everything about reading and writing to connections

        bzero (recvBuff, 4096);
        n = read(connfd, recvBuff, 4096);
        recv_len = strlen(recvBuff);

        printf("\n recvBuff len: %i\n", recv_len);

        if(recv_len > 0)
        {
            printf("\n New message: %s\n", recvBuff);
            ticks = time(NULL);
            //snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
            //snprintf(wrtBuff, sizeof(wrtBuff), "\"%s\" received at %s", recvBuff, sendBuff);
           
            node_t * current = head;
            if(current != NULL)
            {
                if(current->connfd != -1)
                {
                    write(current->connfd, recvBuff, strlen(recvBuff));
                    printf("\n writing to head->connfd: %i with buffer: %s", current->connfd, recvBuff);
                }
                while(current->next != NULL)
                {
                    current = current->next;
                    if(current->connfd != -1)
                    {
                        write(current->connfd, recvBuff, strlen(recvBuff));
                        printf("\n writing to connfd: %i with buffer: %s", current->connfd, recvBuff);
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    /* The function socket() creates a socket in kernel and returns an 
     * integer descriptor of the socket 
     */
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    /* struct sockaddr {
     *     sa_family_t       sa_family;
     *     char              sa_data[14];
     * }
     */

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    
    listen(listenfd, 10);
    
    cli_len = sizeof(cli_addr);
    head = malloc(sizeof(node_t));

    head->connfd = -1;
    head->next = NULL;
    
    while(1)
    {
        // Accept new client connections
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_len);
        if(head->connfd == -1)
        {
            //First connection
            head->connfd = connfd;
        }
        else
        {
            node_t * current = head;
            while(current->next != NULL)
            {
                printf("%d-> ", current->connfd);
                current = current->next;
            }
            current->next = malloc(sizeof(node_t));
            current->next->connfd = connfd;
            current->next->next = NULL;
        }
        
        int err;
        err = pthread_create(&pthr, NULL, &readWrite, NULL);
        if(err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
    }
    return 0;
}
