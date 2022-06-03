#ifndef SERVER_HPP
# define SERVER_HPP

#include "channel.hpp"
#include "user.hpp"
#include <sstream>

class user;
class channel;

class server
{
	public:
		server(std::string key = NULL) : password(key) {};
		~server() {};

		void	printChannels();
		void	printUsers();
		int 	send_message(std::string msg, int userFd);
		int 	parsing(char *input, int userFd);
		void 	createChannel(std::string name, std::string key, int userFd);
		bool 	channelExists(std::string chan);
		int 	join_channel(std::string name, std::string key, int userFd);
		bool 	userExists(int fd);
		int		addUser(int fd);
		channel &findChannel(std::string name);
		user 	&findUser(int userFd);

	private:
		//int port;
		std::string password;
		//int connected_users;
		std::vector<channel> channels;
		std::vector<user> users;
};

#endif