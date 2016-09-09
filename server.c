#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
void err_sys(char* mes);

int main(){
	int sockfd_listen, sockfd_client;
	int clntLen;
	char recvbuf[10];
	struct sockaddr_in addrport, echoClntAddr;
	addrport.sin_family = AF_INET;
	addrport.sin_port = htons(5100);
	addrport.sin_addr.s_addr = htonl(INADDR_ANY);

	if((sockfd_listen = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("Socket open error");

	if(bind(sockfd_listen, (struct sockaddr*)&addrport, sizeof(addrport)) < 0)
		err_sys("Socket bind error");

	if(listen(sockfd_listen, 10) < 0)
		err_sys("Socket listen error");


	/* Repeat: */
//	while(1){
		clntLen = sizeof(echoClntAddr);
		if((sockfd_client = accept(sockfd_listen, (struct sockaddr*)&echoClntAddr, &clntLen)) < 0)
			err_sys("Socket accept error");

		/* Accepted */
		if(recv(sockfd_client, recvbuf, 10, 0) < 0)
			err_sys("Socket recieve error");

		fputs(recvbuf, stdout);

//	}

	if(close(sockfd_listen) < 0)
		err_sys("Socket close error");
}

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}
