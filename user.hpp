#ifndef USER_HPP
# define USER_HPP

#include "channel.hpp"
#include "server.hpp"
#include <cstring>
class channel;

class user
{
	public:
		user(int sock) : fd(sock), command() { memset(buff, 0, 80); };
		~user() {};
		int	const &getFd() const {return fd;};
		channel *currentChan;
		void setCommand(char *buff)
		{
			std::string cmd(buff);

			if (command.find('\n', 0) == std::string::npos)
				command += cmd;
			else
			{
				command.erase(0, command.find('\n', 0) + 1);
				command += cmd;
			}
		};
		std::string getCommand() { return command; };
		char buff[80];
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
