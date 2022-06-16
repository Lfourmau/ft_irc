#include "user.hpp"

void user::setNickname(std::string nick)
{
	if (this->nickname.empty())
	{
		std::string msg(":*!~" + this->username + "@" + this->hostname + " NICK " + ":" + nick + "\n");
		this->nickname = nick;
		if (send(this->fd, msg.data(), msg.length(), 0) < 0)
		{
			perror("  send() failed");
			return ;
		}
		return;
	}
	std::string msg(":" + this->nickname + "!~" + this->username + "@" + this->hostname + " NICK " + ":" + nick + "\n");
	this->nickname = nick;
	if (send(this->fd, msg.data(), msg.length(), 0) < 0)
	{
		perror("  send() failed");
		return ;
	}
	std::cout << "SetNickname called." << std::endl;
}

int user::my_register(std::vector<std::string> &strings)
{
	this->username = strings[1];
	this->realname = strings[4];
	std::cout << "Register called" << std::endl;
	return 0;
}

void user::setCommand(char *buff)
{
	std::string cmd(buff);

	if (command.find("\r\n", 0) == std::string::npos)
		command += cmd;
	else
	{
		command.erase(0, command.find("\r\n", 0) + 2);
		command += cmd;
	}
};

void user::setHostname(sockaddr_in &addr)
{
	this->hostname = inet_ntoa(addr.sin_addr);
	std::cout << this->hostname << std::endl;
}

//Getters
std::string user::getUsername() { return this->username; }
std::string user::getRealname() { return this->realname; }
std::string user::getNickname() { return this->nickname; }
std::string user::getHostname() { return this->hostname; }