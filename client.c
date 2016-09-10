#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

void* receivemsg(void* sockfd){
	int sockfd_client = (int)sockfd;
	char recvbuf[128];

	while(recv(sockfd_client, recvbuf, 128, 0) > 0)
		fputs(recvbuf, stdout);

	return;
}

int main(){
	int sockfd_client, bufLen;
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

	/* Create thread for reveiving */
	if((thread1 = pthread_create(&thread1, NULL, receivemsg, (void*)sockfd_client)) != 0);
		err_sys("Threadcreate error");

	/* Send messages */
	while(fgets(sendbuf, 128, stdin) != NULL){

		/* Exit if client types 'quit' */
		if(!strncmp(sendbuf, "quit", 4))
			break;

		bufLen = strlen(sendbuf);
		if(send(sockfd_client, sendbuf, bufLen, 0) != bufLen)
			err_sys("Socket send error, sent different number of bytes than expected");

	}

	if(close(sockfd_client) < 0)
		err_sys("Socket close error");

	printf("Socket closed succesfully\n");
	return 0;
}
