#ifndef SERVER_HPP
# define SERVER_HPP

#include "channel.hpp"
#include <sstream>

class server
{
	public:
		server(std::string key = NULL) : password(key) {};
		~server() {};
		int 	parsing(std::string toparse);
		void 	createChannel(std::string name, std::string key);
		void	printChannels();
		bool 	channelExists(std::string chan);
		int 	join_channel(std::string name, std::string key);

	private:
		//int port;
		std::string password;
		//int connected_users;
		std::vector<channel> channels;
};

#endif