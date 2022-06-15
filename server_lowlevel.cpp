#include "server_lowlevel.hpp"

Server_lowlevel::Server_lowlevel() : server(), fds_(), listening_socket_(), address_info_(), compress_array_(false) {}

Server_lowlevel::~Server_lowlevel() {
	for (std::vector<struct pollfd>::iterator it = this->fds_.begin(); it != this->fds_.end(); ++it) {
		if (it->fd >= 0)
			close(it->fd);
	}
	this->fds_.clear();
}

void	Server_lowlevel::launch() {

	bool end_server = false;
	
	open_server_connection();
	while (end_server == false) {
		try {
			polling();
			iterate_through_fds();
			compress_array_if_needed();
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
			end_server = true;
		}

	}
}

void	Server_lowlevel::open_server_connection() {

	this->listening_socket_ = socket(AF_INET6, SOCK_STREAM, 0); // AF_INET: ipv4 only; AF_INET6: ipv4 and 6
	if (this->listening_socket_ == -1)
		std::cerr << "Error: socket() fail\n";
	std::cout << "listening socket: " << this->listening_socket_ << std::endl;

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

void	Server_lowlevel::polling() {
	int ret;
	ret = poll(this->fds_.data(), this->fds_.size(), TIMEOUT);
	if (ret == -1)
		throw ServerException("Error: poll() fail");
	else if (ret == 0)
		throw ServerException("poll() timed out. End of program.");
}

void	Server_lowlevel::iterate_through_fds() { 
	for (size_t i = 0; i < this->fds_.size(); ++i){
		if (this->fds_[i].revents == 0)
			continue;
		if (this->fds_[i].revents != POLLIN)
			throw ServerException("Error revents");
		if (this->fds_[i].fd == this->listening_socket_)
			adding_pending_connections();
		else
			receive_and_send_data(i);
	}
}

void	Server_lowlevel::adding_pending_connections() {
	 // adding all pending connections to list of FDs
	std::cout << "Listening socket is readable\n";
	int new_sd = 0;
	struct pollfd new_pollfd;
	while (new_sd != -1) {
		new_sd = accept(this->listening_socket_, NULL, NULL);
		if (new_sd < 0) {
			if (errno != EWOULDBLOCK)
				throw ServerException("accept() failed");
			break;
		}
		std::cout << "New incoming connection - " << new_sd << std::endl;
		new_pollfd.fd = new_sd;
		new_pollfd.events = POLLIN;
		this->fds_.push_back(new_pollfd);
		this->addUser(new_sd); // ----> plugin with high level
	}
}

void	Server_lowlevel::receive_and_send_data( size_t connection_index ) {
	std::cout << "Descriptor" << this->fds_[connection_index].fd << " is readable\n";
	bool close_connection = false;
	char	buffer[BUFFER_SIZE];
	int		return_check;
	int		len;
	while (true) {
		memset(buffer, 0, BUFFER_SIZE);
		//return_check = recv(this->fds_[connection_index].fd, buffer, sizeof(buffer), 0);
		return_check = recv(this->fds_[connection_index].fd,
				this->findUser(this->fds_[connection_index].fd).buff,
				sizeof(this->findUser(this->fds_[connection_index].fd).buff),
				0); // ---> PLUG
		if (return_check == -1) {
			if (errno != EWOULDBLOCK) {
				this->fds_[connection_index].fd = TO_BE_CLOSED;
				this->compress_array_ = true;
				throw ServerException("recv() failed");
			}
			break;
		}
		if (return_check == 0) {
			std::cout << "Connection closed\n";
			this->fds_[connection_index].fd = TO_BE_CLOSED;
			this->compress_array_ = true;
			break;
		}
		len = return_check;
		std::cout << len << " bytes received\n";
		// ---- PLUG High level ----
		this->findUser(this->fds_[connection_index].fd).setCommand(this->findUser(this->fds_[connection_index].fd).buff);
		this->parsing(this->findUser(this->fds_[connection_index].fd).getCommand(), this->fds_[connection_index].fd);
/*		// echo back to the client
		return_check = send(this->fds_[connection_index].fd, buffer, len, 0);
		if (return_check == -1) {
			std::cerr << "send() failed\n";
			this->fds_[connection_index].fd = TO_BE_CLOSED;
			this->compress_array_ = true;
			throw ServerException("send() failed");
		}
*/
	}
	if (close_connection) {
		close(this->fds_[connection_index].fd);
		this->fds_[connection_index].fd = TO_BE_CLOSED;
		this->compress_array_ = true;
	}
}

void	Server_lowlevel::compress_array_if_needed() {
	if (this->compress_array_) {
		this->compress_array_ = false;
		for (std::vector<struct pollfd>::iterator it = this->fds_.begin(); it != this->fds_.end(); ++it) {
			if (it->fd == TO_BE_CLOSED)
				this->fds_.erase(it);
		}
	}
}
