#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int sockfd = 0, n = 0, wrt_len = 0;
char recvBuffer[4096];
char wrtBuffer[4096];
struct sockaddr_in serv_addr;

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usasge: %s <ip of server> \n", argv[0]);
        return 1;
    }

    memset(recvBuffer, '0', sizeof(recvBuffer));
    
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
        printf("Error: Could not create socket \n");
        return 1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("inet_pton error occured\n");
        return 1;
    }
    
    /* connect() takes three parameters
     * sockfd is the descriptor of the client socket in kernel
     * serv_addr is the server socket it connects to (parse into sockaddr ptr)
     * size is the data length of the serv_addr
     */

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error : Connect Failed \n");
        return 1;
    }
    
    if(n < 0)
    {
        printf("Write to socket error\n");
        bzero(wrtBuffer, 4096);
    }
    
    fd_set rfds;
    struct timeval tv;
    int retval;
    
    while(1)
    {
        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        
        /* Wait up to two seconds. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        
        retval = select(sockfd, &rfds, NULL, NULL, &tv);
        
        if (retval == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        else if (retval)
        {
            /* FD_ISSET(0, &rfds) is true so input is available now. */

            /* bzero() is the same as memset(prt*, '0', size) */
            bzero (wrtBuffer, 4096);
            
            /* Read data from stdin using fgets. */
            fgets(wrtBuffer, 4096, stdin);
            printf("\n");
            
            wrt_len = strlen(wrtBuffer);
            
            if(wrt_len > 0 && write(sockfd, wrtBuffer, sizeof(wrtBuffer)) == -1)
            {
                printf("Fail to send message\n");
                close(sockfd);
                return 1;
            }
        }
        else
        {
            // read socket
            bzero(recvBuffer, 4096);
            n = read(sockfd, recvBuffer, sizeof(recvBuffer));
            
            if(n != -1)
                printf("New message: %s\n", recvBuffer);
        }
    }

    close(sockfd);
    return 0;
}
