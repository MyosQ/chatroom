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
void print_welcome(void);
int setup_client_socket(char* address, char* port);

int main(int argc, char* argv[]){
	int sockfd, bufLen;
	char sendbuf[MSGBUFSIZE], welcomerecv[512];
	pthread_t recvthread;

	/* Check input arguments */
	if(argc != 3){
		fprintf(stderr, "Usage: %s <ipaddress> <port>\n", argv[0]);
		return -1;
	}

	/* Connect socket */
	sockfd = setup_client_socket(argv[1], argv[2]);

	/* Print info */
	if(recv(sockfd, welcomerecv, sizeof(welcomerecv), 0) <= 0 )
		err_sys("first recv error");
	print_welcome();
	printf("Users online: \n");
	printf("%s\n", welcomerecv);

	/* New thread for receiving messages */
	if(pthread_create(&recvthread, NULL, recvfunc, (void*)sockfd) != 0)
		err_sys("pthread_create() error");


	/* Send messages */
	while(1){
		printf(">>");
		if((fgets(sendbuf, MSGBUFSIZE, stdin)) == NULL)
			err_sys("error,fgets = null");

		if(!strncmp(sendbuf, "quit\n", 5))
			break;

		bufLen = strlen(sendbuf)+1;
		if(bufLen > 2)
			if(send(sockfd, sendbuf, bufLen, 0) != bufLen){
				fprintf(stderr,"Socket send error, sent different number of bytes than expected\n");
				exit(EXIT_FAILURE);
			}
	}

	if(close(sockfd) < 0)
		err_sys("Socket close error");

	printf("Socket closed successfully.\nBye!\n");
	exit(0);
}

/* Threads recv functioin */
void* recvfunc(void *args){
	int sockfd = (int)args;
	int nbytes;
	char recvbuf[MSGBUFSIZE];

	while(1){
		if((nbytes = recv(sockfd, recvbuf, sizeof(recvbuf), 0)) <= 0){
			if(nbytes < 0)
				err_sys("recv error()");

			err_sys("nothing to receive, server closed on you");
		}
		putchar('\n');
		putchar('\t');
		fputs(recvbuf, stdout);
		putchar('>');
		putchar('>');
		fflush(stdout);
	}
}

/* Setup socket */
int setup_client_socket(char* address, char* port){
	struct addrinfo hints, *res;
	int err, sockfd_client;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((err = getaddrinfo(address, port, &hints, &res)) != 0){
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
	return sockfd_client;
}


/* Welcome info */
void print_welcome(void){
	printf("Welcome to the rPi groupchat!\n\n");
	printf("Here you can send messages to other connected clients.\n");
	printf("Type \"quit\" to quit.\n");
	printf("To get a user alias, type \"username:yourusername\". Then people will know who sent the message\n\n");

}

/* Helping error function */
void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}
