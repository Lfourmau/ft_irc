#include <iostream>
#include <arpa/inet.h> // htons, htonl, ntohs, ntohl, inet_addr
#include <sys/socket.h> // socket
#include <cstring> // memset c++ version

# define MYPORT 4242

int main( int argc, char **argv) {
	(void)argc;
	(void)argv;

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


}
