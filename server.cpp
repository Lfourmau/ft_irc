#include "server.hpp"

/*******************************************************/
/* MAIN FUNCTION                                       */
/*******************************************************/
int server::parsing(std::string toparse, int userFd)
{
	size_t sep = toparse.find("\r\n");
	std::cout << "Need to be parsed : " << toparse << "**" << std::endl;
	while (sep != std::string::npos)
	{
		std::string cmd(toparse.begin(), toparse.begin() + sep);
		std::vector<std::string> strings;
		std::istringstream stream(cmd);
		std::string word;
		while (getline(stream, word, ' '))
			strings.push_back(word);
		if (!strings[0].compare("JOIN"))
			join_channel(userFd, strings);
		else if (!strings[0].compare("NICK"))
			find_user(userFd).set_nickname(strings, *this);
		else if (!strings[0].compare("USER"))
			find_user(userFd).my_register(strings);
		else if (!strings[0].compare("PRIVMSG"))
			send_privmsg(userFd, strings);
		toparse.erase(toparse.begin(), toparse.begin() + sep + 2);
		sep = toparse.find("\r\n", sep + 2);
	}
	if (send_welcome(userFd) < 0)
		return 1;
	//printChannels();
	return 0;
}

/*******************************************************/
/* CHANNEL STUFF                                       */
/*******************************************************/
int server::join_channel(int userFd, std::vector<std::string> &strings)
{

	//std::vector<std::string> channels = parsing_join_input(strings[1]);
	std::vector<std::string> channels = split_string(strings[1], ',');

	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (channel_exists(*it))
			find_channel(*it).add_member(find_user(userFd));
		else
			create_channel(*it, "fake_key", userFd);
		std::string msg(":" + find_user(userFd).get_nickname() + " JOIN " + *it + "\n");
		send_join_notif(msg, *it);
	}
	return 0;
}

std::vector<std::string>	server::parsing_join_input( std::string& channels ) {

	std::vector<std::string> ret_vec;

	size_t start = 0;
	size_t end = 0;
	while ( end != std::string::npos ) {
		end = channels.find_first_of(',', start);
		std::string str = channels.substr(start, end - start);
		ret_vec.push_back(str);
		start = end + 1;
	}	
	return ret_vec;
}

void server::create_channel(std::string name, std::string key, int userFd)
{
	channel toCreate(name, key);
	toCreate.add_member(find_user(userFd));
	channels.push_back(toCreate);
}

/*******************************************************/
/* USER STUFF                            	           */
/*******************************************************/
int server::add_user(int fd, sockaddr_in &addr)
{
	if (user_exists(fd))
		std::cout << "user already connected" << std::endl;
	else
	{
		user toAdd(fd);
		toAdd.set_hostname(addr);
		this->users.push_back(toAdd);
	}
	print_users();
	return 0;
}


/*******************************************************/
/* EXISTS FUNCTION                                     */
/*******************************************************/
bool server::user_exists(int fd)
{
	for (size_t i = 0; i < this->users.size(); i++)
	{
		if (fd == this->users[i].get_fd())
			return true;
	}
	return false;
}
bool	server::user_exists(std::string name)
{
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].get_nickname() == name)
			return true;
	}
	return false;
}
bool server::channel_exists(std::string chan)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (chan == this->channels[i].get_name())
			return true;
	}
	return false;
	
}

/*******************************************************/
/* FIND FUNCTIONS STUFF                                */
/*******************************************************/
channel& server::find_channel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (channels[i].get_name() == name)
			return channels[i];
	}
	//don't know how return because if i call this function, the channel exists
	return (channels[0]);
}
user& server::find_user(int userFd)
{
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].get_fd() == userFd)
			return users[i];
	}
	//don't know how return because if i call this function, the user exists
	return (users[0]);
}


/*******************************************************/
/* SEND FUNCTIONS                                      */
/*******************************************************/
int	server::no_recipient_or_text(int userFd, std::vector<std::string> strings)
{
	if (strings.size() < 2)
	{
		std::string rpl_msg(rpl_string(find_user(userFd), ERR_NORECIPIENT, "No recipient"));
		if (send(userFd, rpl_msg.data(), rpl_msg.length(), 0) < 0)
			perror(" send() failed");
		return 1;
	}
	else if (strings.size() < 3)
	{
		std::string rpl_msg(rpl_string(find_user(userFd), ERR_NOTEXTTOSEND, "No text to send"));
		if (send(userFd, rpl_msg.data(), rpl_msg.length(), 0) < 0)
			perror(" send() failed");
		return 1;
	}
	return 0;
}
std::string server::build_privmsg(int userFd, std::vector<std::string> strings, std::string recipient)
{
	std::string msg(":" + find_user(userFd).get_nickname() + "!~" + find_user(userFd).get_username() + "@" + find_user(userFd).get_hostname() + " PRIVMSG " + recipient);
	//i = 2 because 0and 1 are the command and the recipient
	for (size_t i = 2; i < strings.size(); i++)
	{
		msg += " ";
		msg += strings[i];
	}
	msg += "\n";
	return msg;
}
int server::send_message_to_channel(int userFd, std::string &recipient, std::string msg)
{
	if (!find_channel(recipient).member_exists(find_user(userFd)))
	{
		std::string rpl_message(rpl_string(find_user(userFd), ERR_CANNOTSENDTOCHAN, "Cannot send to channel", recipient));
		if (send(userFd, rpl_message.data(), rpl_message.length(), 0) < 0)
		{
			perror(" Send failed()");
			return -1;
		}
		return 0;
	}
	
	int ret;
	for (size_t i = 0; i < find_channel(recipient).members.size(); i++)
	{
		if (find_channel(recipient).members[i].get_fd() != userFd)
			ret = send(find_channel(recipient).members[i].get_fd(), msg.data(), msg.length(), 0);
		if (ret < 0)
		{
			perror("  send() failed");
			return ret;
		}
	}
	return 0;
}
int server::send_message_to_user(std::string &recipient, std::string msg)
{
	int fd;
	for (size_t i = 0; i < this->users.size(); i++)
	{
		if (users[i].get_nickname() == recipient)
			fd = users[i].get_fd();
	}
	
	int ret = send(fd, msg.data(), msg.length(), 0);
	if (ret < 0)
	{
		perror("  send() failed");
		return ret;
	}
	return 0;
}
int server::send_privmsg(int userFd, std::vector<std::string> &strings)
{
	if (no_recipient_or_text(userFd, strings)) //may be useless because weechat heck args number. but is useful to avoid segfault on next line
		return -1;

	std::vector<std::string> recipients = split_string(strings[1], ',');
	for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end(); it++)
	{
		if (!user_exists(*it) && !channel_exists(*it))
		{
			std::string rpl_message(rpl_string(find_user(userFd), ERR_NOSUCHNICK, "No such nick/channel", *it));
			if (send(userFd, rpl_message.data(), rpl_message.length(), 0) < 0)
			{
				perror(" Send failed()");
				return -1;
			}
			continue; //send rpl and continue to the next recipient
		}
		std::string msg = build_privmsg(userFd, strings, *it);
		if (channel_exists(*it) && send_message_to_channel(userFd, *it, msg) < 0)
			return -1; //send failed, continue to next loop ?
		else if (user_exists(*it) && send_message_to_user(*it, msg) < 0)
			return -1;//send failed, continue to next loop ?
	}
	
	return 0;
}

int server::send_welcome(int userFd)
{
	if (!find_user(userFd).get_nickname().empty() && !find_user(userFd).get_username().empty() && find_user(userFd).is_connected == 0)
	{
		std::string welcome(":" + get_ip() + RPL_WELCOME + find_user(userFd).get_nickname() + " :Welcome to the Ctaleb, Ncatrien and Lfourmau network, " + find_user(userFd).get_nickname() + "!\n");
		if (send(userFd, welcome.data(), welcome.length(), 0) < 0)
			return (-1);
		find_user(userFd).is_connected = 1;
	}
	return (1);
}
int server::send_join_notif(std::string msg, std::string name)
{
	for (size_t i = 0; i < find_channel(name).members.size(); i++)
		send(find_channel(name).members[i].get_fd(), msg.data(), msg.length(), 0);
	return 0;
}


/*******************************************************/
/* GETTERS                       			           */
/*******************************************************/
std::string server::get_ip() { return this->ip; }


/*******************************************************/
/* DEBUG                       			               */
/*******************************************************/
void server::print_channels()
{
	for (size_t i = 0; i < channels.size(); i++)
		std::cout << "Channel--> " << channels[i].get_name() << std::endl;
	
}
void server::print_users()
{
	for (size_t i = 0; i < users.size(); i++)
		std::cout << "User--> " << users[i].get_fd() << std::endl;
	
}
