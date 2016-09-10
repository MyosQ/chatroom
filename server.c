#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#define MESBUFSIZE 128

void err_sys(char* mes);
void *receivemsg(void* sockfd);

int main(){
	pthread_t thread1;
	int sockfd_listen, sockfd_client, bufLen;
	struct sockaddr_in addrport, echoClntAddr;
	socklen_t clntLen = sizeof(echoClntAddr);
	char sendbuf[MESBUFSIZE];

	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(5100);
	addrport.sin_addr.s_addr = htonl(INADDR_ANY);

	if((sockfd_listen = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("Socket open error");
	printf("Socket created succesfully\n");

	if(bind(sockfd_listen, (struct sockaddr*)&addrport, sizeof(addrport)) < 0)
		err_sys("Socket bind error");

	if(listen(sockfd_listen, 10) < 0)
		err_sys("Socket listen error");

	printf("Listening...\n");
	if((sockfd_client = accept(sockfd_listen, (struct sockaddr*)&echoClntAddr, &clntLen)) < 0)
		err_sys("Socket accept error");

	/* Accepted, new thread for recieving msg */
	if (pthread_create(&thread1, NULL, receivemsg, (void*)sockfd_client) != 0)
		err_sys("Pthread_Create error");
	printf("Connection established, type message to client...\n");

	while(1){
		/* Get message from stdin */
		if(fgets(sendbuf, MESBUFSIZE, stdin) == NULL)
			err_sys("error, fgets = null");


		/* Send message */
		bufLen = strlen(sendbuf);
		if(bufLen > 1)
			if(send(sockfd_client, sendbuf, bufLen+1, 0) != bufLen+1){
				fprintf(stderr,"send error, sent different number of bytes than expected\n");
				exit(EXIT_FAILURE);
			}
	}

	if(close(sockfd_listen) < 0)
		err_sys("Socket close error");
	printf("Socket closed succesfully\n");
	return 0;
}

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

void *receivemsg(void* sockfd){
	int sockfd_client = (int)sockfd;
	char recvbuf[MESBUFSIZE], text[] = "Client: \0";

	while(1){
		if(recv(sockfd_client, recvbuf, MESBUFSIZE, 0) <= 0)
			err_sys("Socket recieve error");

		fputs(text, stdout);
		fputs(recvbuf, stdout);
	}
}
