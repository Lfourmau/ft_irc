#ifndef USER_HPP
# define USER_HPP

#include "channel.hpp"
#include "server.hpp"
class channel;

class user
{
	public:
		user(int sock) : fd(sock) {};
		~user() {};
		int	const &getFd() const {return fd;};
		channel *currentChan;

	private:
		int fd;
		std::string command;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string nickname;
		std::string servername;
};

#endif