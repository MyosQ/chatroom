#include "chat.h"

int main(int argc, char* argv[]){
	int sockfd, bufLen;
	char sendbuf[MSGBUFSIZE], users[USERSONLINEBUF]
//	char host[64];
	pthread_t recvthread;

	/* Check input arguments */
	if(argc != 3){
		fprintf(stderr, "Usage: %s <ipaddress> <port>\n", argv[0]);
		return -1;
	}

	/* Get ip if user used hostname */
//	host = look_in_hostfile(argv[1]);

	/* Connect socket */
	sockfd = setup_client_socket(argv[1], argv[2]);

	/* Print welcome info */
	if(recv(sockfd, users, sizeof(users), 0) <= 0 )
		err_sys("first recv error");
	print_welcome(users);

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
		if(bufLen > 2)/*to prevent sending only newline*/
			if(send(sockfd, sendbuf, bufLen, 0) != bufLen)
				fprintf(stderr,"Socket send error, sent different number of bytes than expected\ncontinuing...\n");

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

			printf("nothing to receive, server closed on you\nBye!\n");
			exit(EXIT_SUCCESS);
		}
		recvbuf[MSGBUFSIZE - 1] = '\0';
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
void print_welcome(char* users){
	printf("Welcome to the epic RPi-hosted groupchat!\n\n");
	printf("Here you can send messages to other connected clients.\n");
	printf("To get a user alias, type \"username:yourusername\". Then people will know who sent the message\n");
	printf("Type \"quit\" to quit.\n\n");
	printf("Users online right now: \n");
	printf("%s\n", users);
}

/* Helping error function */
void err_sys(char* mes){
	perror(mes);
	fflush(stdout);
	exit(EXIT_FAILURE);
}
