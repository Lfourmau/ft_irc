#ifndef SERVER_HPP
#	define SERVER_HPP

#	include <iostream>
#	include <arpa/inet.h> // htons, htonl, ntohs, ntohl, inet_addr
#	include <sys/socket.h> // socket
#	include <cstring> // memset c++ version
#	include <poll.h>
#	include <cerrno>
#	include <unistd.h>
#	include <fcntl.h>

#	include <vector>

class Server {

	public:

		void	launch();

	private:
		
		std::vector<struct pollfd>	fds_;	
		int							listening_socket_;

		void	open_connection(); // socket(), bind(), setsockopt(), fcntl(), listen()
		void	

};

#endif
