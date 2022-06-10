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
#	include <exception>
#	include <string>

#	include <vector>

# define MYPORT 4242
# define BACKLOG 64
# define MAX_CONNECTIONS 1024
# define BUFFER_SIZE 80
# define TIMEOUT	180000

class Server {

	public:

		Server();
		~Server();

		void	launch();

		class ServerException : public std::exception {
			std::string message;
			ServerException( std::string str ) : message(str) {}
			~ServerException () throw() {}
			const char* what() const throw() { return (message.c_str()); }
		};

	private:
		
		std::vector<struct pollfd>	fds_;
		int							listening_socket_;
		struct	sockaddr_in6		address_info_;

		void	open_server_connection(); // socket(), bind(), setsockopt(), fcntl(), listen()
		void	polling(); // poll;
		void	adding_pending_connections(); // accept()
		void	receive_and_send_data(); // recv(), sendv()
		void	iterate_through_fds() // wtf??

		Server( Server& other );
		Server&	operator=( Server& other );
};

#endif
