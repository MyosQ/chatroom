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
#include <fcntl.h>

#define MSGBUFSIZE 256
#define MAXUSERS 10
#define USERNAMELEN 20
#define USERSONLINEBUF 512
#define LEFTCHATBUF 64

/*ser*/
void *receivemsg(void* sockfd);
int socket_initialize(char *port);
void handleTcpClient(int sockfd);
int get_peer_info(int sockfd, char* host, char* service);
void update_fd_max(int fd, int* fd_max);
int accept_request(int sockfd_listen, fd_set *connected_fds, int *fd_max);
void set_username(int i, char* msgbuf, int nbytes, char** usernamearray);
void send_users_online(int newfd, int listener, fd_set connected_fds, int fd_max, char** usernames);

/*cli*/
void* recvfunc(void *args);
void print_welcome(char* users);
int setup_client_socket(char* address, char* port);

void err_sys(char* mes);
