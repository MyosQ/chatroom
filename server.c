#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

void err_sys(char* mes);
void *receivemsg(void* sockfd);

int main(){
	pthread_t thread1;
	int sockfd_listen, sockfd_client;
	struct sockaddr_in addrport, echoClntAddr;
	socklen_t clntLen = sizeof(echoClntAddr);

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
	char sendbuf[128];
	while(fgets(sendbuf, 128, stdin) != NULL){

		if(send(sockfd_client, sendbuf, strlen(sendbuf), 0) != strlen(sendbuf))
			err_sys("Server send error");

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
	char recvbuf[10];

	printf("Receiving...\n");
	while(1){
		if(recv(sockfd_client, recvbuf, 10, 0) <= 0)
			err_sys("Socket recieve error");

		fputs(recvbuf, stdout);
	}
}
