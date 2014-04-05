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

typedef struct node {
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
int listenfd = 0, connfd = 0;
int n;
socklen_t recv_len, cli_len;

char wrtBuff[4096];
char recvBuff[4096];
time_t ticks;
pthread_t pthr;

void readWrite(int readfd) {
	// Everything about reading and writing to connections

	bzero(recvBuff, 4096);
	n = read(readfd, recvBuff, 4096);
	recv_len = strlen(recvBuff);

	printf("\n recvBuff len: %i\n", recv_len);

	if (recv_len > 0) {
		printf("\n New message: %s\n", recvBuff);

		if (head != NULL) {
			if (head->connfd != readfd) {
				write(head->connfd, recvBuff, strlen(recvBuff));
				printf("\n writing to head->connfd: %i with buffer: %s",
						head->connfd, recvBuff);
			}
			node_t * current = head;
			while (current->next != NULL) {
				current = current->next;
				if (readfd != current->connfd) {
					write(current->connfd, recvBuff, strlen(recvBuff));
					printf("\n writing to current->connfd: %i with buffer: %s",
							current->connfd, recvBuff);
				}
			}
		}
	}
}

void * selectRead(void * arg) {
	fd_set rfds, sfds;
	struct timeval tv;
	int retval;

	if (head != NULL) {
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		FD_ZERO(&rfds);
		FD_SET(head->connfd, &rfds);

		if (select(head->connfd + 1, &rfds, NULL, NULL, &tv)) {
			readWrite(head->connfd);
		}

		if (head->next != NULL) {
			node_t * current = head;
			while (current->next != NULL) {
				current = current->next;
				FD_ZERO(&rfds);
				FD_SET(current->connfd, &rfds);
				if (select(current->connfd + 1, &rfds, NULL, NULL, &tv)) {
					readWrite(current->connfd);
				}
			}
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	/* The function socket() creates a socket in kernel and returns an
	 * integer descriptor of the socket
	 */

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

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

	// Accept new client connections
	while ((connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len))) {
		if (head == NULL) {
			//First connection
			head = malloc(sizeof(node_t));
			head->connfd = connfd;
			head->next = NULL;
			printf("first connection: %d", head->connfd);
		} else {
			node_t * current = head;
			while (current->next != NULL) {
				printf("%d-> ", current->connfd);
				current = current->next;
			}
			current->next = malloc(sizeof(node_t));
			current->next->connfd = connfd;
			current->next->next = NULL;
		}

		int err;
		err = pthread_create(&pthr, NULL, &selectRead, NULL);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));

	}
	return 0;
}
