#include "user.hpp"

bool	is_valid_nickname(std::string& nick) {
	std::string start_check = INVALID_STARTCHARS;

	if (nick.length() > MAX_NICK_LENGTH)
		return false;
	if (start_check.find(nick.data(), 0, 1) != std::string::npos)
		return false;
	if (nick.find_first_of(INVALID_INCHARS) != std::string::npos)
		return false;
	return true;
}


int user::my_register(std::vector<std::string> &strings)
{
	this->username = strings[1];
	this->realname = strings[4];
	return 0;
}

/*******************************************************/
/* SETTERS 		                                       */
/*******************************************************/
int user::set_command(char *buff)
{
	std::string cmd(buff);

	if (command.find_last_of("\r\n") == std::string::npos)
	{
		command += cmd;
		if (command.find_last_of("\r\n", 0))
			return 1;
		return 0;
	}
	else
	{
		command.erase(0, command.find_last_of("\r\n") + 2);
		command += cmd;
		if (command.find_last_of("\r\n") == std::string::npos)
			return 0;
		return 1;
	}

};
int user::set_nickname(std::vector<std::string> &strings, server& server)
{
	std::string nick = strings[1];
	std::cout << "nickname: " << nick << std::endl;
	
	if (strings.size() < 2) {
		std::string rpl_message(rpl_string(*this, ERR_NONICKNAMEGIVEN, "No nickname given"));
		if (send(this->get_fd(), rpl_message.data(), rpl_message.length(), 0) < 0) {
			perror("  send() failed");
			return -1;
		}
		return 0;
	}
	else if (strings.size() > 2 || !is_valid_nickname(nick)) {
		std::string rpl_message(rpl_string(*this, ERR_ERRONEUSNICKNAME, "Erroneous nickname", nick));
		if (send(this->get_fd(), rpl_message.data(), rpl_message.length(), 0) < 0) {
			perror("  send() failed");
			return -1;
		}
		return 0;
	}
	else if (server.find_user(nick)) {
		std::string rpl_message(rpl_string(*this, ERR_NICKNAMEINUSE, "Nickname is already in use", nick));
		if (send(this->get_fd(), rpl_message.data(), rpl_message.length(), 0) < 0) {
			perror("  send() failed");
			return -1;
		}
		return 0;
	}
	std::string msg(":" + this->nickname + "!~" + this->username + "@" + this->hostname + " NICK " + ":" + nick + "\n");
	this->nickname = nick;
	if (send(this->fd, msg.data(), msg.length(), 0) < 0)
	{
		perror("  send() failed");
		return -1;
	}
	return 0;
}
void user::set_hostname(sockaddr_in &addr)
{
	this->hostname = inet_ntoa(addr.sin_addr);
	std::cout << this->hostname << std::endl;
}

/*******************************************************/
/* GETTERS 		                                       */
/*******************************************************/
std::string user::get_username() const { return this->username; }
std::string user::get_realname() const { return this->realname; }
std::string user::get_nickname() const { return this->nickname; }
std::string user::get_hostname() const { return this->hostname; }