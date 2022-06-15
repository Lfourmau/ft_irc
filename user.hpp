#ifndef USER_HPP
# define USER_HPP

#include "channel.hpp"
#include "server.hpp"
class channel;

class user
{
	public:
		user(int sock) : fd(sock), command(), nickname() { memset(buff, 0, 80); };
		~user() {};

		channel *currentChan;
		char buff[80];

		int	const 	&getFd() const {return fd;};
		std::string getCommand() { return command; };
		std::string getUsername();
		std::string getRealname();
		std::string getNickname();
		std::string getHostname();
		void 		setCommand(char *buff);
		void 		setNickname(std::string nick);
		void 		setHostname(sockaddr_in &addr);

		int	my_register(std::vector<std::string> &strings);
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