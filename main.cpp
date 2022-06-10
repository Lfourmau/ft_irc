#include <iostream>
#include <arpa/inet.h> // htons, htonl, ntohs, ntohl, inet_addr
#include <sys/socket.h> // socket
#include <cstring> // memset c++ version
#include <poll.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

# define MYPORT 4242
# define BACKLOG 64
# define MAX_CONNECTIONS 1024
# define BUFFER_SIZE 80

int main( int argc, char **argv) {
	(void)argc;
	(void)argv;

	bool compress_array = false;
	char buffer[BUFFER_SIZE];
	int len;
	int sockfd;
	struct	sockaddr_in6	my_addr;
	memset(&my_addr, 0, sizeof(my_addr));

	sockfd = socket(AF_INET6, SOCK_STREAM, 0); // AF_INET: ipv4 only; AF_INET6: ipv4 and 6
	if (sockfd == -1)
		std::cerr << "Error: socket() fail\n";
	std::cout << "sockfd: " << sockfd << std::endl;

	my_addr.sin6_family = AF_INET6;
	my_addr.sin6_port = htons(MYPORT);
	my_addr.sin6_addr = in6addr_any; // global variable

	if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in6)) == -1)
		std::cerr << "Error: bind() fail\n";
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		std::cerr << "Error: setsockopt() fail\n";
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		std::cerr << "Error: fcntl() fail\n";
	if (listen(sockfd, BACKLOG) == -1)	
		std::cerr << "Error: listen() fail\n";

	struct pollfd fds[MAX_CONNECTIONS];
	int	nfds = 1;
	memset(fds, 0, sizeof(fds));
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	int timeout = (3 * 60 * 1000);
	int	return_check;
	
	bool end_server = false;
	int current_size = 0;
	int close_connection;
	while (end_server == false) {
		std::cout << "waiting on poll()...\n";
		return_check = poll(fds, nfds, timeout);
		if (return_check == -1) {
			std::cerr << "Error: poll() fail\n";
			break;
		}
		else if (return_check == 0) {
			std::cerr << " poll() timed out. End of program.\n";
			break;
		}
		current_size = nfds;
		for (int i = 0; i < current_size; ++i) {
			if (fds[i].revents == 0)
				continue;
			if (fds[i].revents != POLLIN) {
				std::cerr << "Error! revents = " << fds[i].revents << std::endl;
				end_server = true;
				break;
			}
			if (fds[i].fd == sockfd) { // adding all pending connections to list of FDs
				std::cout << "Listening socket is readable\n";
				int new_sd = 0;
				while (new_sd != -1) {
					new_sd = accept(sockfd, NULL, NULL);
					if (new_sd < 0) {
						if (errno != EWOULDBLOCK) {
							std::cerr << " accept() failed\n";
							end_server = true;
						}
						break;
					}
					std::cout << "New incoming connection - " << new_sd << std::endl;
					fds[nfds].fd = new_sd;
					fds[nfds].events = POLLIN;
					++nfds;
				}
			}
			else { // not the listening (server) socket
				std::cout << "Descriptor" << fds[i].fd << " is readable\n";
				close_connection = false;
				while (true) {
					memset(buffer, 0, BUFFER_SIZE);
					return_check = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (return_check == -1) {
						if (errno != EWOULDBLOCK) {
							std::cerr << "recv() failed\n";
							close_connection = true;
						}
						break;
					}
					if (return_check == 0) {
						std::cout << "Connection closed\n";
						close_connection = true;
						break;
					}
					len = return_check;
					std::cout << len << " bytes received\n";
					// echo back to the client
					return_check = send(fds[i].fd, buffer, len, 0);
					if (return_check == -1) {
						std::cerr << "send() failed\n";
						close_connection = true;
						break;
					}
				}

				if (close_connection) {
					close(fds[i].fd);
					fds[i].fd = -1;
					compress_array = true;
				}

			} // end of existing connection
		} // end of loop through poll

		if (compress_array) {
			compress_array = false;
			for (int i = 0; i < nfds; ++i) {
				if (fds[i].fd == -1) {
					for (int j = i; j < nfds; ++j)
						fds[j].fd = fds[j+1].fd;
					--i;
					--nfds;
				}
			}
		}
	} // end of server running
	
	// clean up all open sockets
	for (int i = 0; i < nfds; ++i) {
		if (fds[i].fd >= 0)
			close(fds[i].fd);
	}
}
