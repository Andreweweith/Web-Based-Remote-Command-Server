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
#include "CommandLineHandler.h"
#include "HttpHeaderInterpreter.h"
#include "structs.h"
#include "URIDecoder.h"
#include "CommandHandler.h"
#include "JsonOutput.h"

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
	//printf("Opening socket %d\n", sockfd);
	char buf[RECV_BUFFER_SIZE];
	int nBytes;
	while ((nBytes = recv(sockfd, buf, RECV_BUFFER_SIZE - 1, 0)) > 0) {
		buf[nBytes] = '\0';
		printf("%s\n", buf);
		struct httpheader *httpheader_ = getHttpHeaderStruct(buf);
		char* requestType = malloc(strlen(httpheader_->firstline) * sizeof(char));
		char* uri = malloc(strlen(httpheader_->firstline) * sizeof(char));
		char* garbage = malloc(strlen(httpheader_->firstline) * sizeof(char));
		sscanf(httpheader_->firstline, "%s %s %s", requestType, uri, garbage);
		struct input *input_ = decode_uri(uri);

		if (strcmp(requestType, "GET") == 0) {
			if (strcmp(input_->path, "/") == 0) {
				input_->path = "/webserver.html";
			}

			input_->path++;
			FILE *fp = fopen(input_->path, "r");
			if (fp != NULL) {
				fseek(fp, 0, SEEK_END);
				long int size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				char *content = malloc((size + 1) * sizeof(char));
				fread(content, 1, size, fp);
				fclose(fp);
				char *content_type;
				char *dot = strchr(input_->path, '.');
				if (strcmp(dot, ".css") == 0) {
					content_type = "text/css";
				} else if (strcmp(dot, ".js") == 0) {
					content_type = "text/javascript";
				} else {
					content_type = "text/html";
				}

				char *format =
					"HTTP/1.1 200 OK\n"
					"Content-Type: %s\n"
					"Content-Length: %d\n"
					"\n"
					"%s";
				char *reply = malloc((size + 100) * sizeof(char));
				sprintf(reply, format, content_type, strlen(content) - 1, content);
				if (send(sockfd, reply, strlen(reply), 0) == -1) {
					perror("send");
				}
			} else {
				char *reply404 =
					"HTTP/1.1 404 Not Found\n"
					"Content-Type: text/html\n"
					"Content-Length: 23\n"
					"\n"
					"HTTP 404 Page Not Found";
				printf("Unable to send requested resource %s\n", input_->path);
				if (send(sockfd, reply404, strlen(reply404), 0) == -1) {
					perror("send");
				}
			}
		} else if (strcmp(requestType, "POST") == 0) {
			struct input *input_ = decode_uri(uri);
			if (strcmp(input_->path, "/exec") == 0) {
				struct output *output_ = handle_command(input_->cmd);
				char *outputString = getOutputJsonString(output_);
				char *format =
					"HTTP/1.1 200 OK\n"
					"Content-Type: application/json\n"
					"Content-Length: %d\n"
					"\n"
					"%s";
				char *reply = malloc((strlen(outputString) + 100) * sizeof(char));
				sprintf(reply, format, strlen(outputString), outputString);
				if (send(sockfd, reply, strlen(reply), 0) == -1) {
					perror("send");
				}
			}
		}
	}

	/*if ((nBytes = recv(sockfd, buf, RECV_BUFFER_SIZE - 1, 0)) > 0) {
		buf[nBytes] = '\0';
		printf("I SHOULD NOT GET THIS\n%s\n", buf);
	}*/
	if (nBytes == -1) {
		perror("recv");
	}
	//printf("Closing socket %d\n", sockfd);
	close(sockfd);
}

int main(int argc, char *argv[]) {
	struct commandlineoptions *options = malloc(sizeof(struct commandlineoptions));
	if (!handleCommandLineOptions(argc, argv, options)) {
		return 0;
	}
	char *port = options->port;
	int restrictConnections = options->restrictConnections;
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

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
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

		/*if (restrictConnections == 1) {
			char ipaddress[INET_ADDRSTRLEN];
			struct sockaddr *client_sockaddr = (struct sockaddr *)&their_addr;
			if (client_sockaddr->sa_family == AF_INET) {
				inet_ntop(AF_INET, &((struct sockaddr_in *)client_sockaddr)->sin_addr, ipaddress, INET_ADDRSTRLEN);
				if (strcmp(ipaddress, "127.0.0.1") != 0) {
					char *reply =
						"HTTP/1.1 403 Forbidden\n"
						"Content-Type: text/html\n"
						"Content-Length: 18\n"
						"\n"
						"HTTP 403 Forbidden";
					printf("server: connection refused to %s\n", ipaddress);
					if (send(sockfd, reply, strlen(reply), 0) == -1) {
						perror("send");
					}
					close(new_fd);
					continue;
				}
			} else if (client_sockaddr->sa_family == AF_INET6) {
				inet_ntop(AF_INET6, &((struct sockaddr_in6 *)client_sockaddr)->sin6_addr, ipaddress, INET_ADDRSTRLEN);
				if (strcmp(ipaddres, "::1") != 0) {
					char *reply =
						"HTTP/1.1 403 Forbidden\n"
						"Content-Type: text/html\n"
						"Content-Length: 18\n"
						"\n"
						"HTTP 403 Forbidden";
					printf("server: connection refused to %s\n", ipaddress);
					if (send(sockfd, reply, strlen(reply), 0) == -1) {
						perror("send");
					}
					close(new_fd);
					continue;
				}
			}

		}*/

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (restrictConnections == 1) {
			if (strcmp(s, "127.0.0.1") != 0 && strcmp(s, "::1") != 0) {
				char *reply =
					"HTTP/1.1 403 Forbidden\n"
					"Content-Type: text/html\n"
					"Content-Length: 18\n"
					"\n"
					"HTTP 403 Forbidden";
				printf("server: connection refused to %s\n", s);
				if (send(sockfd, reply, strlen(reply), 0) == -1) {
					perror("send");
				}
				close(new_fd);
				continue;
			}
		}

		if (!fork()) {
			HandleConnection(new_fd);
			exit(0);
		}
	}
	return 0;
}