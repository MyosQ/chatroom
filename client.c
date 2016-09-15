#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#define MSGBUFSIZE 256

void err_sys(char* mes);
void* recvfunc(void *args);

int main(int argc, char* argv[]){
	struct addrinfo hints, *res;
	int sockfd_client, err, bufLen;
	char sendbuf[MSGBUFSIZE];
	pthread_t recvthread;

	if(argc != 3){
		fprintf(stderr, "Usage: %s <ipaddress> <port>\n", argv[0]);
		return -1;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
 		exit(EXIT_FAILURE);
	}

	if((sockfd_client = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
		freeaddrinfo(res);
		err_sys("socket error");
	}
	if(connect(sockfd_client, res->ai_addr, res->ai_addrlen) != 0){
		freeaddrinfo(res);
		err_sys("connect error");
	}
	freeaddrinfo(res);



	/* New thread for receiving messages */
	if(pthread_create(&recvthread, NULL, recvfunc, (void*)sockfd_client) != 0)
		err_sys("pthread_create() error");

	/* Send messages */
	printf("Type messages to other clients. Type \"quit\" to quit.\n");
	while(1){
		if((fgets(sendbuf, MSGBUFSIZE, stdin)) == NULL)
			err_sys("error,fgets = null");

		if(!strncmp(sendbuf, "quit", 4))
			break;

		bufLen = strlen(sendbuf)+1;
		if(bufLen > 2)
			if(send(sockfd_client, sendbuf, bufLen, 0) != bufLen){
				fprintf(stderr,"Socket send error, sent different number of bytes than expected\n");
				exit(EXIT_FAILURE);
			}
	}

	if(close(sockfd_client) < 0)
		err_sys("Socket close error");

	printf("Socket closed successfully\n");
	exit(0);
}

void* recvfunc(void *args){
	int sockfd = (int)args;
	int nbytes;
	char recvbuf[MSGBUFSIZE];

	while(1){
		if((nbytes = recv(sockfd, recvbuf, sizeof(recvbuf), 0)) <= 0)
			err_sys("recv error, server probably closed on you");

		putchar('\t');
		fputs(recvbuf, stdout);
	}
}



void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}
