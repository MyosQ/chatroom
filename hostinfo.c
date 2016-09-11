#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ERRBUFSIZE 128

void err_quit(char* msg);
void print_flags(struct addrinfo *p);
void print_family(struct addrinfo *p);
void print_socktype(struct addrinfo *p);
void print_protocol(struct addrinfo *p);

int main(int argc, char* argv[]){
	char errbuf[ERRBUFSIZE], addrbuf[INET_ADDRSTRLEN], addrbuf6[INET6_ADDRSTRLEN];
	struct addrinfo *ailist, *p, hints;
	int err;
	const char* address;
	struct sockaddr_in* sinp;
	struct sockaddr_in6* sinp6;

	if(argc != 3){
		fprintf(stderr, "Usage: %s <nodename> <service>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;

	/* Get info */
	if((err = getaddrinfo(argv[1], argv[2], &hints, &ailist)) != 0){
		sprintf(errbuf,"getaddrinfo error: %s", gai_strerror(err));
		err_quit(errbuf);
	}

	/* Loop through info list and print */
	for(p = ailist; p != NULL; p = p->ai_next){
		printf("Host: %s\n", p->ai_canonname?p->ai_canonname:"-");
		if(p->ai_family == AF_INET){
			sinp = (struct sockaddr_in*)p->ai_addr;
			address = inet_ntop(AF_INET, &sinp->sin_addr, addrbuf, INET_ADDRSTRLEN);
			printf("Address: %s  ", address?address:"unknown");
			printf("Port: %d  ", ntohs(sinp->sin_port));
		}
		if(p->ai_family == AF_INET6){
			sinp6 = (struct sockaddr_in6*)p->ai_addr;
			address = inet_ntop(AF_INET6, &sinp6->sin6_addr, addrbuf6, INET6_ADDRSTRLEN);
			printf("Address: %s  ", address?address:"unknown");
			printf("Port: %d  ", ntohs(sinp->sin_port));
		}
		print_flags(p);
		print_family(p);
		print_socktype(p);
		print_protocol(p);
		printf("\n\n");
	}

	freeaddrinfo(ailist);
	return 0;
}

void print_protocol(struct addrinfo *p){
	printf("Protocol:");
	switch(p->ai_protocol){
		case 0:
			printf(" default");
			break;
		case IPPROTO_TCP:
			printf(" TCP");
			break;
		case IPPROTO_UDP:
			printf(" UDP");
			break;
		case IPPROTO_RAW:
			printf(" raw");
			break;
		default:
			printf(" unknown (%d)", p->ai_protocol);
			break;
	}
	printf("  ");
}
void print_socktype(struct addrinfo *p){
	printf("Socket type:");
	switch(p->ai_socktype){
		case SOCK_DGRAM:
			printf(" datagram");
			break;
		case SOCK_STREAM:
			printf(" stream");
			break;
		case SOCK_SEQPACKET:
			printf(" seqpacket");
			break;
		case SOCK_RAW:
			printf(" raw");
			break;
		default:
			printf(" unknown (%d)", p->ai_socktype);
	}
	printf("  ");
}
void print_family(struct addrinfo *p){
	printf("Family:");
	switch(p->ai_family){
		case AF_INET:
			printf(" inet");
			break;
		case AF_INET6:
			printf(" inet6");
			break;
		case AF_UNIX:
			printf(" unix");
			break;
		case AF_UNSPEC:
			printf(" unspecified");
			break;
		default:
			printf(" unknown");
			break;
	}
	printf("  ");
}

void print_flags(struct addrinfo *p){
	printf("Flags:");
	if(p->ai_flags == 0)
		printf(" 0");
	else{
		if(p->ai_flags & AI_PASSIVE)
			printf(" passive");
		if(p->ai_flags & AI_CANONNAME)
			printf(" canon");
		if(p->ai_flags & AI_NUMERICHOST)
			printf(" numhost");
		if(p->ai_flags & AI_NUMERICSERV)
			printf(" numserv");
		if(p->ai_flags & AI_V4MAPPED)
			printf(" v4mapped");
		if(p->ai_flags & AI_ALL)
			printf(" allIP");
	}
	printf("  ");
}

void err_quit(char* msg){
	perror(msg);
	fflush(NULL);
	exit(1);
}
