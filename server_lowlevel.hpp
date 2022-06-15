#ifndef SERVER_LOWLEVEL_HPP
#	define SERVER_LOWLEVEL_HPP

#	include <iostream>
#	include <arpa/inet.h> // htons, htonl, ntohs, ntohl, inet_addr
#	include <sys/socket.h> // socket
#	include <cstring> // memset c++ version
#	include <poll.h>
#	include <cerrno>
#	include <unistd.h>
#	include <fcntl.h>
#	include <exception>
#	include <string>

#	include <vector>

# include "server.hpp"

# define MYPORT 4242
# define BACKLOG 64
# define MAX_CONNECTIONS 1024
# define BUFFER_SIZE 80
# define TIMEOUT	180000
# define TO_BE_CLOSED (-1)

class Server_lowlevel : public server {

	public:

		Server_lowlevel();
		~Server_lowlevel();

		void	launch();

		class ServerException : public std::exception {
			std::string message;
			public:
				ServerException( std::string str ) : message(str) {}
				~ServerException () throw() {}
				const char* what() const throw() { return (message.c_str()); }
		};

	private:
		
		std::vector<struct pollfd>	fds_;
		int							listening_socket_;
		struct	sockaddr_in6		address_info_;
		bool						compress_array_;

		void	open_server_connection(); // socket(), bind(), setsockopt(), fcntl(), listen()
		void	polling(); // poll;
		void	adding_pending_connections(); // accept()
		void	receive_and_send_data( size_t connection_index ); // recv(), sendv()
		void	iterate_through_fds(); // wtf??
		void	compress_array_if_needed();

		Server_lowlevel( Server_lowlevel& other );
		Server_lowlevel&	operator=( Server_lowlevel& other );
};

#endif
