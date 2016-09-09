#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

int main(){
	int sockfd_client;
	struct sockaddr_in echoServAddr;
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr("192.168.1.15");
	echoServAddr.sin_port = htons(5100);
	char sendbuf[128];

	if((sockfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		err_sys("Socket create error");

	if(connect(sockfd_client, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0)
		err_sys("Socket connect error");


	/* Send messages */
	while(fgets(sendbuf, 128, stdin) != NULL){

		if(send(sockfd_client, sendbuf, 128, 0) != 128)
			err_sys("Socket send error, different number of bytes than expected");

	}

	if(close(sockfd_client) < 0)
		err_sys("Socket close error");

	return 0;
}
