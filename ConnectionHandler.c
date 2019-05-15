#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "HttpHeaderInterpreter.h"
#include "structs.h"
#include "URIDecoder.h"

#define PORT "3838"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define RECV_BUFFER_SIZE 10000

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

			 // waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void HandleConnection(int socket) {
	int sockfd = socket;
	char buf[RECV_BUFFER_SIZE];
	int nBytes;
	if ((nBytes = recv(sockfd, buf, RECV_BUFFER_SIZE - 1, 0)) > 0) {
		buf[nBytes] = '\0';
		printf("%s\n", buf);
		struct httpheader *httpheader_ = getHttpHeaderStruct(buf);
		char* requestType = malloc(strlen(httpheader_->firstline) * sizeof(char));
		char* uri = malloc(strlen(httpheader_->firstline) * sizeof(char));
		char* garbage = malloc(strlen(httpheader_->firstline) * sizeof(char));
		sscanf(httpheader_->firstline, "%s %s %s", requestType, uri, garbage);
		printf("%s\n", uri);

		if (strcmp(requestType, "GET") == 0) {
			if (strcmp(uri, "/") == 0) {
				// send monster file
				char *reply =
					"HTTP/1.1 200 OK\n"
					"Content-Type: text/html\n"
					"Content-Length: 15\n"
					"Accept-Ranges: bytes\n"
					"Connection: keep-alive\n"
					"\n"
					"sdfkjsdnbfkjbsf";
				if (send(sockfd, reply, strlen(reply), 0) == -1) {
					perror("send");
				}
			} else {
				char *reply404 =
					"HTTP/1.1 404 Bad\n"
					"Content-Type: text/html\n"
					"Content-Length: 23\n"
					"Accept-Ranges: bytes\n"
					"\n"
					"HTTP 404 Page Not Found";
				if (send(sockfd, reply404, strlen(reply404), 0) == -1) {
					perror("send");
				}
			}
		}
		//if (strlen(uri) > 0) {
		//	struct input *input_ = decode_uri(uri);
		//}
	}
	if (nBytes == -1) {
		perror("recv");
	}
	close(sockfd);
}

int main(int argc, char *argv[]) {
	int sockfd; // listen on sockfd
	int new_fd; // new connections on new_fd
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		perror("getaddrinfo");
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while (1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) {
			HandleConnection(new_fd);
			exit(0);
		}
	}
	return 0;
}