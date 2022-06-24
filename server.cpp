#include "server.hpp"

server::~server()
{
	for (std::vector<user*>::iterator it = users.begin(); it != users.end(); ++it)
		delete (*it);
}

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
			find_user(userFd)->set_nickname(strings, *this);
		else if (!strings[0].compare("USER"))
			find_user(userFd)->my_register(strings);
		else if (!strings[0].compare("PRIVMSG"))
			send_privmsg(userFd, strings);
		else if (!strings[0].compare("KICK"))
			kick(userFd, strings);
		else if (!strings[0].compare("PART"))
			part(userFd, strings);
		toparse.erase(toparse.begin(), toparse.begin() + sep + 2);
		sep = toparse.find("\r\n", sep + 2);
	}
	if (send_welcome(userFd) < 0)
		return 1;
	//printChannels();
	return 0;
}

/*******************************************************/
/* PART STUFF        	                               */
/*******************************************************/
int server::part(int userFd, std::vector<std::string>& strings)
{
	std::vector<std::string> recipients = split_string(strings[1], ',');
	for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end(); ++it)
	{
		if (!channel_exsists(*it))
		{
			std::string rpl_msg = rpl_string(kicker, ERR_NOSUCHCHANNEL, "No such channel", chan_name);
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			continue ;
		}
		if (!find_channel(*it).member_exists(find_user(userFd)))
		{
			std::string rpl_msg = rpl_string(kicker, ERR_NOTONCHANNEL, "You're not on that channel", chan_name);
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			continue ;
		}
		find_channel(*it).remove_member(find_user(userFd));
	}
	
}


/*******************************************************/
/* KICK STUFF        	                               */
/*******************************************************/
int server::fin_and_send_kick_rpl(int userFd, std::string chan_name, std::string nickname)
{
	user *kicker = find_user(userFd);
	if (!channel_exists(chan_name))
	{
		std::string rpl_msg = rpl_string(kicker, ERR_NOSUCHCHANNEL, "No such channel", chan_name);
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	channel &chan = find_channel(chan_name);
	if (!chan.member_exists(nickname))
	{
		std::string rpl_msg = rpl_string(kicker, ERR_USERNOTINCHANNEL, "They aren't on that channel", nickname, chan_name);
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	if (!chan.member_exists(kicker->get_nickname()))
	{
		std::string rpl_msg = rpl_string(kicker, ERR_NOTONCHANNEL, "You're not on that channel", chan_name);
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	return 0;
}
int server::kick(int userFd, std::vector<std::string>& strings)
{
	//if there channel does not exists, the find channel return chan[0]. Need to fix this. 
	std::string chan_name = strings[1];
	std::string nickname = strings[2];
	std::string reason;
	if (fin_and_send_kick_rpl(userFd, chan_name, nickname))
		return -1;
	channel &chan = find_channel(chan_name);
	chan.remove_member(chan.find_member(nickname));
	if (strings.size() >= 4)
		for (size_t i = 3; i < strings.size(); ++i)
			reason.append(strings[i] + " ");
	std::string msg(":" + find_user(userFd)->get_nickname() + " KICK " + chan_name + " " + nickname + " :" + reason + "\n");
	chan.send_to_members(msg);
	return 0;
}


/*******************************************************/
/* CHANNEL STUFF                                       */
/*******************************************************/
int server::join_channel(int userFd, std::vector<std::string> &strings)
{
	std::vector<std::string> channels = split_string(strings[1], ',');

	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (!channel_exists(*it))
			create_channel(*it, "fake_key");
		find_channel(*it).add_member(find_user(userFd));
		if (send_join_rpl(*it, userFd) < 0)
			return -1;
	}
	return 0;
}

void server::create_channel(std::string name, std::string key)
{
	channel toCreate(name, key);
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
		user *toAdd = new user(fd);
		toAdd->set_hostname(addr);
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
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if (fd == (*it)->get_fd())
			return true;
	}
	return false;
}
bool	server::user_exists(std::string name)
{
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if ((*it)->get_nickname() == name)
			return true;
	}
	return false;
}
bool server::channel_exists(std::string chan)
{
	for (std::vector<channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		if (chan == it->get_name())
			return true;
	}
	return false;
}

/*******************************************************/
/* FIND FUNCTIONS STUFF                                */
/*******************************************************/
channel& server::find_channel(std::string name)
{
	for (std::vector<channel>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		if (it->get_name() == name)
			return *it;
	}
	//don't know how return because if i call this function, the channel exists
	return (channels[0]);
}
user* server::find_user(int userFd)
{
	user* ret = NULL;
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if ((*it)->get_fd() == userFd)
		{
			ret = (*it);
			break;
		}
	}
	return (ret);
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
	std::string msg(":" + find_user(userFd)->get_nickname() + "!~" + find_user(userFd)->get_username() + "@" + find_user(userFd)->get_hostname() + " PRIVMSG " + recipient);
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
	channel &chan = find_channel(recipient);
	if (!chan.member_exists(*(find_user(userFd))))
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
	for (std::vector<user*>::iterator it = chan.members.begin(); it != chan.members.end(); ++it)
	{
		if ((*it)->get_fd() != userFd)
			ret = send((*it)->get_fd(), msg.data(), msg.length(), 0);
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
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if ((*it)->get_nickname() == recipient)
			fd = (*it)->get_fd();
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
	user *new_user = find_user(userFd);
	if (!new_user->get_nickname().empty() && !new_user->get_username().empty() && new_user->is_connected == 0)
	{
		std::string welcome(":" + get_ip() + RPL_WELCOME + new_user->get_nickname() + " :Welcome to the Ctaleb, Ncatrien and Lfourmau network, " + new_user->get_nickname() + "!\n");
		if (send(userFd, welcome.data(), welcome.length(), 0) < 0)
			return (-1);
		new_user->is_connected = 1;
	}
	return (1);
}
int server::send_join_notif(std::string msg, std::string name)
{
	channel &chan = find_channel(name);
	for (std::vector<user*>::iterator it = chan.members.begin(); it != chan.members.end(); ++it)
		send((*it)->get_fd(), msg.data(), msg.length(), 0);
	return 0;
}

int server::send_join_rpl(std::string channel_name, int userFd)
{
	channel &chan = find_channel(channel_name);
	user *new_user = find_user(userFd);

	std::string msg(":" + new_user->get_nickname() + " JOIN " + channel_name + "\n");
	send_join_notif(msg, channel_name);

	//send_rpl_namreply
	std::string namreply(":" + get_ip() + RPL_NAMREPLY + new_user->get_nickname() + " = " + channel_name + " :");
	for (std::vector<user*>::iterator it_user = chan.members.begin(); it_user != chan.members.end(); ++it_user)
		namreply.append((*it_user)->get_nickname() + " ");
	namreply.append("\n");
	std::cerr << "namreply: " << namreply << std::endl;
	if (send(userFd, namreply.data(), namreply.length(), 0) < 0)
		return -1;
	//send_rpl_endofnames
	std::string endofnames(":" + get_ip() + RPL_ENDOFNAMES + new_user->get_nickname() + " " + channel_name + " :End of /NAMES list.\n");
	if (send(userFd, endofnames.data(), endofnames.length(), 0) < 0)
		return -1;
	return 0;
}

/*******************************************************/
/* GETTERS                       			           */
/*******************************************************/
std::string server::get_ip() { return this->ip; }
std::vector<user*> &server::get_users() { return this->users; }
std::vector<channel> &server::get_channels() { return this->channels; }


/*******************************************************/
/* DEBUG                       			               */
/*******************************************************/
void server::print_channels()
{
	for (std::vector<channel>::iterator it = channels.begin(); it != channels.end(); ++it)
		std::cout << "Channel--> " << it->get_name() << std::endl;
}
void server::print_users()
{
	for (std::vector<user*>::iterator it = users.begin(); it != users.end(); ++it)
		std::cout << "User--> " << (*it)->get_fd() << " nickname: " << (*it)->get_nickname() << std::endl;
}
