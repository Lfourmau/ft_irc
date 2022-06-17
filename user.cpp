#include "user.hpp"

int user::init_nickname(std::string nick)
{
	std::string msg(":*!~" + this->username + "@" + this->hostname + " NICK " + ":" + nick + "\n");
	this->nickname = nick;
	if (send(this->fd, msg.data(), msg.length(), 0) < 0)
	{
		perror("  send() failed");
		return -1;
	}
	return 0;
}
int user::set_nickname(std::string nick)
{
	if (this->nickname.empty())
		return (init_nickname(nick));
	std::string msg(":" + this->nickname + "!~" + this->username + "@" + this->hostname + " NICK " + ":" + nick + "\n");
	this->nickname = nick;
	if (send(this->fd, msg.data(), msg.length(), 0) < 0)
	{
		perror("  send() failed");
		return -1;
	}
	return 0;
}

int user::my_register(std::vector<std::string> &strings)
{
	this->username = strings[1];
	this->realname = strings[4];
	return 0;
}

void user::set_command(char *buff)
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

void user::set_hostname(sockaddr_in &addr)
{
	this->hostname = inet_ntoa(addr.sin_addr);
	std::cout << this->hostname << std::endl;
}

/*******************************************************/
/* GETTERS 		                                       */
/*******************************************************/
std::string user::get_username() { return this->username; }
std::string user::get_realname() { return this->realname; }
std::string user::get_nickname() { return this->nickname; }
std::string user::get_hostname() { return this->hostname; }