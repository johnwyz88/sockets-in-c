#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd = 0, n = 0, wrt_len = 0;
char recvBuff[1024];
char wrtbuffer[4096];
struct sockaddr_in serv_addr;
pthread_t pthr[2];
typedef enum { false, true }  bool;
bool sendBool;

void * readConnection(void * arg)
{
    if((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {   
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    }

    if(n < 0)
    {
        printf("\n Read error \n");
    }
    return NULL;
}

void * userInput(void * arg)
{
    while(1)
    {        
        printf("\n Enter your message: \n");
        
        /* bzero() is the same as memset(prt*, '0', size) */
        bzero (wrtbuffer, 4096);
        fgets(wrtbuffer, 4096, stdin);
        if(!strcmp(wrtbuffer.toString().trim(), "switch"))
        {
            printf("sendBool = false");
            sendBool = false;
            break;
        }
        if(sendBool)
        {
            wrt_len = strlen(wrtbuffer);
            printf("\n wrt_len: %i", (int)wrt_len);

            if(wrt_len > 0)
            {
                // send is equivalent to write with a 0 flag
                send(sockfd, wrtbuffer, sizeof(wrtbuffer),0);
            }   
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("\n Usasge: %s <ip of server> \n", argv[0]);
        return 1;
    }

    memset(recvBuff, '0', sizeof(recvBuff));
    
    /* No binding is needed for the client socket
     * because it is free to use any port assigned by
     * the kernel.
     * It is the opposite for the server because
     * the socket needs to be well known.
     * Therefore known server is bind to a specific port 
     * like http server runs on port 80.
     */

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error: Could not create socket \n");
        return 1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }
    
    /* connect() takes three parameters
     * sockfd is the descriptor of the client socket in kernel
     * serv_addr is the server socket it connects to (parse into sockaddr ptr)
     * size is the data length of the serv_addr
     */

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    if(n < 0)
    {
        printf("\n Write to socket error\n");
        bzero(wrtbuffer, 4096);
    }
    
    sendBool = true;

    while(1)
    {
        int err;
        if(sendBool)
        {
            userInput(NULL);
            //err = pthread_create(&pthr[0], NULL, &userInput, NULL);
            //if(err != 0)
            //printf("\ncan't create thread: [%s]",strerror(err));
        }
        else
        {
            printf("read connection");
            readConnection(NULL);
            //err = pthread_create(&pthr[1], NULL, &readConnection, NULL);
            //if(err != 0)
            //    printf("\ncan't create thread: [%s]",strerror(err));
    
        }
    }
    return 0;
}
