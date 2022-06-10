#include "server.hpp"

Server::Server() : fds_(), listening_socket_(), address_info_() {}

Server::~Server() { this->fds_.clear() }

void	Server::launch() {

	bool end_server = false;
	
	open_server_connection();
	while (end_server == false) {
		try {
			polling();
			iterate_through_fds();
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			end_server = true;
		}

	}
}

void	Server::open_server_connection() {

	this->listening_socket_ = socket(AF_INET6, SOCK_STREAM, 0); // AF_INET: ipv4 only; AF_INET6: ipv4 and 6
	if (this->listening_socket_ == -1)
		std::cerr << "Error: socket() fail\n";
	std::cout << "sockfd: " << sockfd << std::endl;

	this->address_info_.sin6_family = AF_INET6;
	this->address_info_.sin6_port = htons(MYPORT);
	this->address_info_.sin6_addr = in6addr_any; // global variable

	if (bind(this->listening_socket_, (struct sockaddr*)&this->address_info_, sizeof(this->address_info_)) == -1)
		std::cerr << "Error: bind() fail\n";
	int yes = 1;
	if (setsockopt(this->listening_socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
		std::cerr << "Error: setsockopt() fail\n";
	if (fcntl(this->listening_socket_, F_SETFL, O_NONBLOCK) == -1)
		std::cerr << "Error: fcntl() fail\n";
	if (listen(this->listening_socket_, BACKLOG) == -1)	
		std::cerr << "Error: listen() fail\n";
	
	struct pollfd initial_listening;
	initial_listening.fd = this->listening_socket_;
	initial_listening.events = POLLIN;
	this->fds_.push_back(initial_listening);
}

void	Server::polling() {
	int ret;
	ret = poll(this->fds.data(), this->fds.size(), TIMEOUT);
	if (ret == -1)
		throw ServerException("Error: poll() fail");
	else if (ret == 0)
		throw ServerException("poll() timed out. End of program.");
}

void	Server::iterate_through_fds() { 
	for (std::vector<struct pollfd>::iterator it = fds_.begin(); it != fds_.end(); ++it){
		if (it->revents == 0)
			continue;
		if (it->revents != POLLIN)

	}
}
