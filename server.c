#include "chat.h"

int main(int argc, char *argv[]){
	int sockfd_listen, fd_max = 0, i, j, nbytes, totalLen, newfd;
	fd_set read_fds, connected_fds;
	char msgbuf[MSGBUFSIZE], userandmsg[MSGBUFSIZE];
	char** usernamearray;

	/* Check arguments */
	if(argc != 2){
		fprintf(stderr,"Usage: %s <port>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Zero fds's */
	FD_ZERO(&read_fds);
	FD_ZERO(&connected_fds);

	/* initialize socket */
	sockfd_listen = socket_initialize(argv[1]);

	/* set listener socket into fd-set */
	FD_SET(sockfd_listen, &connected_fds);
	update_fd_max(sockfd_listen, &fd_max);


	/* Allocate username array */
	if((usernamearray = calloc(MAXUSERS, sizeof(char*))) == NULL)
		err_sys("Couldnt allocate memory");
	for(i = 0; i < MAXUSERS; i++)
		if((usernamearray[i] = calloc(USERNAMELEN, sizeof(char))) == NULL)
			err_sys("Couldnt allocate memory");

	/* Deal with readable sockets */
	while(1){
		read_fds = connected_fds;
		if(select(fd_max+1, &read_fds, NULL, NULL, NULL) < 0)
			err_sys("select() error");

		for(i = 0; i <= fd_max; i++){
			if(FD_ISSET(i, &read_fds)){
				if(i == sockfd_listen){

					/* Deal with new connection */
					newfd = accept_request(i, &connected_fds, &fd_max);
					send_users_online(newfd, i, connected_fds, fd_max, usernamearray);
				}
				else{
					/* Deal with message from connected client */
					if((nbytes = recv(i, msgbuf, sizeof(msgbuf), 0)) <= 0){
						if(nbytes == 0)
							printf("connection closed on socket %d\n", i);
						else
							perror("recv() error");

						close(i);
						FD_CLR(i, &connected_fds);
						memset(usernamearray[i], 0, USERNAMELEN);
					}
					else{
						if(!strncmp(msgbuf, "username:", 9)){
							set_username(i, msgbuf, nbytes, usernamearray);
						}
						else{
							for(j = 0; j <= fd_max; j++){
								if(FD_ISSET(j, &connected_fds) && j != i && j != sockfd_listen){
									/* Build message */
									memset(userandmsg, 0, sizeof(userandmsg));
									if(usernamearray[i][0] != 0){
										strcpy(userandmsg, usernamearray[i]);
										strcat(userandmsg, ": ");
										totalLen = strlen(usernamearray[i]) + nbytes + 2;
									}
									else{
										strcpy(userandmsg, "Anonymous: ");
										totalLen = 11 + nbytes + 2;
									}
									strcat(userandmsg, msgbuf);
									if(send(j, userandmsg, totalLen, 0) != totalLen)
										perror("send() error");
								}
							}
						}
					}
				}
			}/*ifFD_ISSET*/
		}/*For*/
	}/*While*/
	return 0;
}


/*Function that sends info about connect hosts to newly connected client */
void send_users_online(int newfd, int listener, fd_set connected_fds, int fd_max, char** usernames){
	int k;
	char host[64], service[64], useronline[128], totalmsg[512];
	memset(&totalmsg, 0, sizeof(totalmsg));

	/* Build the total message */
	for(k = 0; k <= fd_max; k++){
		if(k == newfd)
			strncat(totalmsg,"You\n", 4);

		else if(FD_ISSET(k, &connected_fds) && k != newfd && k != listener){
			memset(&useronline, 0, sizeof(useronline));
			if(get_peer_info(k, host, service) != 0){
				snprintf(useronline, sizeof(useronline),"Couldnt get hostinfo on socket %d\n", k);
			}
			else{
				snprintf(useronline, sizeof(useronline), "%s %s %s %s\n", "Host:", host, "Username:", (usernames[k][0]) ? usernames[k] : "Unknown.");
			}
			strcat(totalmsg, useronline);
		}
	}
	totalmsg[strlen(totalmsg)] = '\0';
	send(newfd, totalmsg, sizeof(totalmsg), 0);
}

/* Accept incoming request and print some information
   Returns new sockfd */
int accept_request(int sockfd_listen, fd_set *connected_fds, int *fd_max){
	int sockfd_client;
	char host[64], service[64];
	struct sockaddr_storage clientAddr;
	socklen_t addr_size = sizeof(struct sockaddr_in);

	if((sockfd_client = accept(sockfd_listen, (struct sockaddr*)&clientAddr, &addr_size)) < 0){
		perror("Server accept error");
		return -1;
	}
	else{
		FD_SET(sockfd_client, connected_fds);
		update_fd_max(sockfd_client, fd_max);
		get_peer_info(sockfd_client, host, service);
		printf("New connected client on socket %d.  ", sockfd_client);
		printf("host: %s, service: %s\n", host, service);
		return sockfd_client;
	}
}


/* Create and bind socket (only server)*/
int socket_initialize(char *port){
	char host[64], service[64], hostname[64];
	int sockfd_listen, err;
	struct addrinfo hints, *result, *p;
	struct sockaddr_in ret;
	socklen_t sockaddrsize = sizeof(struct sockaddr_in);

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

	freeaddrinfo(result);
	if(p == NULL){
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	/* Get some binding info */
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


/* Print info about peer using its socket */
int get_peer_info(int sockfd, char* host, char* service){
	struct sockaddr addr;
	socklen_t size = sizeof(addr);

	if(getpeername(sockfd, &addr, &size) < 0){
		fprintf(stderr,"couldnt get peer info");
		return -1;
	}
	if(getnameinfo(&addr, size, host, 64, service, 64, 0) != 0){
		fprintf(stderr,"getnameinfo error");
		return -1;
	}
	return 0;
}

/* Not being used in this program */
void handleTcpClient(int sockfd){
	int ret;
	int sockfd_client = sockfd;
	char recvbuf[MSGBUFSIZE];

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

/* Insert username into array */
void set_username(int i, char* msgbuf, int nbytes, char** usernamearray){
	nbytes -= 9; /* get rid of first part */
	if(nbytes > USERNAMELEN)
		nbytes = USERNAMELEN;

	nbytes -= 2;/* not newline nor null */
	memcpy((void*)usernamearray[i], &msgbuf[9], nbytes);
	usernamearray[i][nbytes] = '\0';
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
