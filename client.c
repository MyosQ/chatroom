#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}

int main(){
	int sockfd_client;
	struct sockaddr_in echoServAddr;
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	echoServAddr.sin_port = htons(5100);


	if((sockfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		err_sys("Socket create error");

	if(connect(sockfd_client, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0)
		err_sys("Socket connect error");

	char* echoString = "Hola!\n";
	int echoStrLen = strlen(echoString);

	if(send(sockfd_client, echoString, echoStrLen, 0) != echoStrLen)
		err_sys("Socket send error, different number of bytes than expected");

	if(close(sockfd_client) < 0)
		err_sys("Socket close error");
}
