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
#define MESBUFSIZE 128

void err_sys(char* mes);

int main(int argc, char* argv[]){
	struct addrinfo hints, *res;
	int sockfd_client, err;
	char sendbuf[MESBUFSIZE];
	int bufLen;

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

	/* Send messages */
	printf("Type messages:\n");
	while(1){

		if((fgets(sendbuf, MESBUFSIZE, stdin)) == NULL)
			err_sys("error,fgets = null");

		if(!strncmp(sendbuf, "quit", 4))
			break;

		bufLen = strlen(sendbuf);
		if(bufLen > 1)
			if(send(sockfd_client, sendbuf, bufLen+1, 0) != bufLen+1){
				fprintf(stderr,"Socket send error, sent different number of bytes than expected\n");
				exit(EXIT_FAILURE);
			}
	}

	if(close(sockfd_client) < 0)
		err_sys("Socket close error");
	printf("Socket closed successfully\n");

	exit(0);
}

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}
