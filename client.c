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

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    char wrtbuffer[4096];
    struct sockaddr_in serv_addr;

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

    while(1)
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

        printf("\n Enter your message: \n");
        
        /* bzero() is the same as memset(prt*, '0', size) */
        bzero (wrtbuffer, 4096);
        fgets(wrtbuffer, 4096, stdin);

        n = write(sockfd, wrtbuffer, strlen(wrtbuffer));
        if(n < 0)
        {
            printf("\n Write to socket error\n");
            bzero(wrtbuffer, 4096);
        }
        else
        {
            send(sockfd, wrtbuffer, sizeof(wrtbuffer),0);
        }
    }
    return 0;
}
