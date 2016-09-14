#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#define MSGBUFSIZE 256

void err_sys(char* mes);
void *receivemsg(void* sockfd);
int socket_initialize(char *port);
void handleTcpClient(int sockfd);
int print_peer_info(int sockfd);
void childreaper(int signo);
void update_fd_max(int fd, int* fd_max);

int main(int argc, char *argv[]){
	int sockfd_listen, sockfd_client, fd_max = 0, i, j, nbytes;
	struct sockaddr_storage clientAddr;
	socklen_t addr_size;
	addr_size = sizeof(clientAddr);
	fd_set read_fds, connected_fds;
	char msgbuf[MSGBUFSIZE];

	FD_ZERO(&read_fds);
	FD_ZERO(&connected_fds);

	if(argc != 2){
		fprintf(stderr,"Usage: %s <port>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Signal handler */
	signal(SIGCHLD, &childreaper);

	/* initialize socket */
	sockfd_listen = socket_initialize(argv[1]);

	/* fds-set */
	FD_SET(sockfd_listen, &connected_fds);
	update_fd_max(sockfd_listen, &fd_max);

	/* Deal with incoming connections */
	while(1){
		read_fds = connected_fds;
		if(select(fd_max+1, &read_fds, NULL, NULL, NULL) < 0)
			err_sys("select() error");

		for(i = 0; i <= fd_max; i++){
			if(FD_ISSET(i, &read_fds)){
				if(i == sockfd_listen){
					if((sockfd_client = accept(sockfd_listen, (struct sockaddr*)&clientAddr, &addr_size)) < 0)
						perror("Server accept error");
					else{
						FD_SET(sockfd_client, &connected_fds);
						update_fd_max(sockfd_client, &fd_max);
						printf("New connection on socket %d\n", sockfd_client);
					}
				}
				else{
					if((nbytes = recv(i, msgbuf, sizeof(msgbuf), 0)) <= 0){
						if(nbytes == 0)
							printf("connection closed on socket %d\n", i);
						else
							perror("recv() error");

						close(i);
						FD_CLR(i, &connected_fds);
					}
					else{
						for(j = 0; j <= fd_max; j++){
							if(FD_ISSET(j, &connected_fds) && j != i && j != sockfd_listen){
								if(send(j, msgbuf, nbytes, 0) != nbytes)
									perror("send() error");
							}
						}
					}
				}
			}/*ifFD_ISSET*/
		}/*For*/
	}/*While*/
	return 0;
}

/* Create and bind socket */
int socket_initialize(char *port){
	int sockfd_listen, err;
	struct addrinfo hints, *result, *p;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((err = getaddrinfo(NULL, port, &hints, &result)) != 0){
		fprintf(stderr,"getaddrinfo error: %s\n\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Try to bind each address until we succeed */
	for(p = result; p != NULL; p = p->ai_next){
		if((sockfd_listen = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
			continue;

		if(bind(sockfd_listen, p->ai_addr, p->ai_addrlen) == 0)
			break;

		close(sockfd_listen);
	}

	if(p == NULL){
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);

	/* Get some binding info */
	char host[64], service[64], hostname[64];
	struct sockaddr_in ret;
	socklen_t sockaddrsize = sizeof(struct sockaddr_in);

	if(getsockname(sockfd_listen, (struct sockaddr*)&ret, &sockaddrsize) != 0){
		close(sockfd_listen);
		err_sys("getsockname error");
	}
	if(getnameinfo((struct sockaddr*)&ret, sockaddrsize, host, sizeof(host), service, sizeof(service), NI_NUMERICHOST) != 0){
		close(sockfd_listen);
		err_sys("getnameinfo error");
	}
	gethostname(hostname, sizeof(hostname));

	/* Print that info */
	printf("Binding successful!\n");
	printf("Hostname: %s", hostname);
	printf("\tHostaddress: %s", host);
	printf("\tService: %s\n", service);

	/* Set up for listening */
	if(listen(sockfd_listen, 10) < 0)
		err_sys("listen error");

	puts("Listening...\n\n");
	return sockfd_listen;
}



int print_peer_info(int sockfd){
	char host[64], service[64];
	struct sockaddr addr;
	socklen_t size = sizeof(addr);
	printf("New connected client:");

	if(getpeername(sockfd, &addr, &size) < 0){
		fprintf(stderr,"couldnt get peer info");
		return -1;
	}
	if(getnameinfo(&addr, size, host, 64, service, 64, NI_NUMERICHOST) != 0){
		fprintf(stderr,"getnameifo error");
		return -1;
	}
	printf(" %s", host);
	printf(" %s\n\n", service);

	return 0;
}

void handleTcpClient(int sockfd){
	int ret;
	int sockfd_client = sockfd;
	char recvbuf[MSGBUFSIZE];

	print_peer_info(sockfd_client);

	while(1){
		if((ret = recv(sockfd_client, recvbuf, sizeof(recvbuf), 0)) <= 0){
			if(ret == 0){
				fprintf(stderr,"Client left you\n");
				exit(0);
			}
			err_sys("Server recieve error");
		}
		fputs(recvbuf, stdout);
	}
}

/* Smooth helper function */
void update_fd_max(int fd, int* fd_max){
	if(fd > *fd_max)
		*fd_max = fd;
}

/* Signal handling function */
void childreaper(int signo){
	wait(NULL);
}

/* Error function */
void err_sys(char* mes){
	perror(mes);
	exit(EXIT_FAILURE);
}
