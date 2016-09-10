#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MESBUFSIZE 128

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

void* receivemsg(void* sockfd){
	int sockfd_client = (int)sockfd;
	char recvbuf[MESBUFSIZE], text[9] = "Server: \0";

	while(1){
		if(recv(sockfd_client, recvbuf, MESBUFSIZE, 0) <= 0)
			err_sys("Receive error");

		fputs(text,stdout);
		fputs(recvbuf, stdout);
	}
}

int main(){
	int sockfd_client, bufLen, ret;
	struct sockaddr_in echoServAddr;
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr("192.168.1.15");
	echoServAddr.sin_port = htons(5100);
	char sendbuf[128];
	pthread_t thread1;

	if((sockfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		err_sys("Socket create error");
	printf("Socket created...\n");

	if(connect(sockfd_client, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0)
		err_sys("Socket connect error");
	printf("Connection established\n");

	/* Create thread for receiving */
	if((ret = pthread_create(&thread1, NULL, receivemsg, (void*) sockfd_client)) != 0)
		err_sys("Threadcreate error");


	/* Send messages */
	while(1){

		/* Get message */
		if((fgets(sendbuf, MESBUFSIZE, stdin)) == NULL)
			err_sys("error,fgets = null");

		/* Exit if client types 'quit' */
		if(!strncmp(sendbuf, "quit", 4))
			break;

		bufLen = strlen(sendbuf);
		printf("buflen : %d ", bufLen);
		printf("Lastchar :%d\n", sendbuf[bufLen]);
		printf("nastLastchar :%d\n", sendbuf[bufLen-1]);
		if(bufLen > 1)
			if(send(sockfd_client, sendbuf, bufLen+1, 0) != bufLen+1){
				fprintf(stderr,"Socket send error, sent different number of bytes than expected\n");
				exit(EXIT_FAILURE);
			}
	}

	/* Close socket */
	if(close(sockfd_client) < 0)
		err_sys("Socket close error");
	printf("Socket closed succesfully\n");

	return 0;
}
