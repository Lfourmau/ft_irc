#ifndef SERVER_HPP
# define SERVER_HPP

#include "channel.hpp"
#include "user.hpp"
#include <sstream>

enum mode
{
	INVITE_ONLY,
	OTHER_MODE,
};

class user;
class channel;

class server
{
	public:
		server(std::string key = NULL) : password(key) {};
		~server() {};

		void	print_channels();
		void	print_users();
		int 	send_message(std::string msg, int userFd);
		int 	parsing(std::string toparse, int userFd);
		void 	create_channel(std::string name, std::string key, int userFd);
		bool 	channel_exists(std::string chan);
		int 	join_channel(int userFd, std::string name, std::string key);
		bool 	user_exists(int fd);
		int		add_user(int fd, sockaddr_in &addr);
		channel &find_channel(std::string name);
		user 	&find_user(int userFd);
		int		send_welcome(int userFd);

	private:
		//int port;
		std::string password;
		//int connected_users;
		std::vector<channel> channels;
		std::vector<user> users;
};

#endif