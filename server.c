#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#define MESBUFSIZE 128
#define PORT_LISTEN 5100
#define MAX_PENDING 10

void err_sys(char* mes);
void *receivemsg(void* sockfd);
int socket_initialize();
void handleTcpClient(int sockfd);

int main(){
	pid_t pid;
	int sockfd_listen, sockfd_client;
	struct sockaddr_in clientAddr;
	socklen_t clntLen = sizeof(clientAddr);

	/* initialize socket */
	sockfd_listen = socket_initialize();

	/* Accept incoming connections */
	while(1){
		if((sockfd_client = accept(sockfd_listen, (struct sockaddr*)&clientAddr, &clntLen)) < 0)
			err_sys("Socket accept error");

		/* Accepted, create process for recieving msg */
		if((pid = fork()) < 0)
			err_sys("fork error");
		else if(pid == 0){
			close(sockfd_listen);
			handleTcpClient(sockfd_client);
		}
		else
			close(sockfd_client);
	}
	return 0;
}

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

void handleTcpClient(int sockfd){
	int sockfd_client = sockfd;
	char recvbuf[MESBUFSIZE], text[] = "Client: \0";

	while(1){
		if(recv(sockfd_client, recvbuf, MESBUFSIZE, 0) <= 0)
			err_sys("Socket recieve error");

		fputs(text, stdout);
		fputs(recvbuf, stdout);
	}
}
int socket_initialize(){
	int sockfd_listen;
	struct sockaddr_in addrport;
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(PORT_LISTEN);
	addrport.sin_addr.s_addr = htonl(INADDR_ANY);

	if((sockfd_listen = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("Socket open error");
	printf("Socket created succesfully\n");

	if(bind(sockfd_listen, (struct sockaddr*)&addrport, sizeof(addrport)) < 0)
		err_sys("Socket bind error");

	if(listen(sockfd_listen, MAX_PENDING) < 0)
		err_sys("Socket listen error");

	printf("Listening...\n");

	return sockfd_listen;
}
