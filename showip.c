#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char* argv[]){

	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	void* addr;
	char* ipver;
	struct sockaddr_in *ipv4;
	struct sockaddr_in6 *ipv6;

	if(argc != 2){
		fprintf(stderr,"Usage: %s <hostname>\n", argv[0]);
		return(1);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0){
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(status));
		return(2);
	}
	int i = 0;
	printf("IP addresses for %s\n\n", argv[1]);

	for(p = res; p != NULL; p = p->ai_next){
		if(p->ai_family == AF_INET){
			ipv4 = (struct sockaddr_in*)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		}
		else{
			ipv6 = (struct sockaddr_in6*)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";

		}
		inet_ntop(p->ai_family, addr, ipstr, INET6_ADDRSTRLEN);
		printf("  %s: %s\n", ipver, ipstr);
		i++;
	}
	printf("%d\n", i);
	freeaddrinfo(res);
	return 0;
}
