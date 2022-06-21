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
			send_message(userFd, strings);
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
	if (channel_exists(strings[1]))
		find_channel(strings[1]).add_member(find_user(userFd));
	else
		create_channel(strings[1], strings[2], userFd);
	std::string msg(":" + find_user(userFd).get_nickname() + " JOIN " + strings[1] + "\n");
	send_join_alert(msg, strings[1]);
	//send(userFd, msg.data(), msg.length(), 0);
	//findChannel(name).printMembers();
	return 0;
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
bool	server::find_user(std::string name)
{
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].get_nickname() == name)
			return true;
	}
	return false;
}


/*******************************************************/
/* SEND FUNCTIONS                                      */
/*******************************************************/
std::string server::build_message(int userFd, std::vector<std::string> strings)
{
	std::string msg(":" + find_user(userFd).get_nickname() + "!~" + find_user(userFd).get_username() + "@" + find_user(userFd).get_hostname() + " PRIVMSG " + strings[1]);
	//i = 2 because 0and 1 are the command and the recipient
	for (size_t i = 2; i < strings.size(); i++)
	{
		msg += " ";
		msg += strings[i];
	}
	msg += "\n";
	return msg;
}
int server::send_message_to_channel(int userFd, std::vector<std::string> &strings, std::string msg)
{
	int ret;

	for (size_t i = 0; i < find_channel(strings[1]).members.size(); i++)
	{
		if (find_channel(strings[1]).members[i].get_fd() != userFd)
			ret = send(find_channel(strings[1]).members[i].get_fd(), msg.data(), msg.length(), 0);
		if (ret < 0)
		{
			perror("  send() failed");
			return ret;
		}
	}
	return 0;
}
int server::send_message_to_user(std::vector<std::string> &strings, std::string msg)
{
	int fd;
	for (size_t i = 0; i < this->users.size(); i++)
	{
		if (users[i].get_nickname() == strings[1])
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
int server::send_message(int userFd, std::vector<std::string> &strings)
{
	if (!user_exists(userFd) && !channel_exists(strings[1]))
		return -1;
	std::string msg = build_message(userFd, strings);
	if (channel_exists(strings[1]) && send_message_to_channel(userFd, strings, msg) < 0)
		return -1;
	else if (find_user(strings[1]) && send_message_to_user(strings, msg) < 0)
		return -1;
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
int server::send_join_alert(std::string msg, std::string name)
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