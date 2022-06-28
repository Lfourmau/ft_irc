#include "server.hpp"

server::server(int port, std::string key ) : port(port), password(key), ip("10.1.8.2") {} 

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
		if (strings.empty())
			return 0;
		if (!strings[0].compare("JOIN"))
			join_channel(userFd, strings);
		else if (!strings[0].compare("NICK"))
			find_user(userFd)->set_nickname(strings, *this);
		else if (!strings[0].compare("USER"))
			find_user(userFd)->my_register(strings);
		else if (!strings[0].compare("PRIVMSG"))
			send_privmsg(userFd, strings);
		else if (!strings[0].compare("NOTICE"))
			send_notice(userFd, strings);
		else if (!strings[0].compare("TOPIC"))
			topic(userFd, strings);
		else if (!strings[0].compare("KICK"))
			kick(userFd, strings);
		else if (!strings[0].compare("MODE"))
			change_mode(userFd, strings);
		else if (!strings[0].compare("INVITE"))
			invitation(userFd, strings);
		else if (!strings[0].compare("PART"))
			part(userFd, strings);
		else if (!strings[0].compare("LIST"))
			list(userFd, strings);
		else if (!strings[0].compare("PING"))
			pong(userFd, strings);
		else if (!strings[0].compare("QUIT"))
			return QUIT;
		toparse.erase(toparse.begin(), toparse.begin() + sep + 2);
		sep = toparse.find("\r\n", sep + 2);
	}
	if (find_user(userFd) && find_user(userFd)->is_connected == 0 && send_welcome(userFd) < 0)
		return 1;
	//printChannels();
	return 0;
}

/*******************************************************/
/* TOPIC STUFF        	                               */
/*******************************************************/
int server::topic(int userFd, std::vector<std::string>& strings)
{
	user *command_author = find_user(userFd);
	if (!channel_exists(strings[1]))
		return -1;
	channel &chan = find_channel(strings[1]);

	if (!chan.member_exists(command_author->get_nickname()))
	{
		std::string rpl_msg = rpl_string(command_author, ERR_NOTONCHANNEL, "You're not on that channel", chan.get_name());
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	//user try to modify topic
	if (strings.size() > 2)
	{
		//he needs privileges
		if (!chan.is_operator(command_author->get_nickname()))
		{	std::string rpl_msg = rpl_string(command_author, ERR_CHANOPRIVSNEEDED, "You're not channel operator", chan.get_name());
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			return -1;
		}
		else
			set_topic(userFd, chan, strings);
	}
	else if (strings.size() == 2)
	{
		//user wants to check the topic
		if (chan.get_topic() != "")
		{
			std::string rpl_msg = rpl_string(command_author, RPL_TOPIC, chan.get_topic().data(), chan.get_name());
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		}
		else
		{
			std::string rpl_msg = rpl_string(command_author, RPL_NOTOPIC, "No topic is set", chan.get_name());
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		}
	}
	return 0;
}

void server::set_topic(int userFd, channel &chan, std::vector<std::string> strings)
{
	std::string new_topic;
	user *modifier = find_user(userFd);

	for (size_t i = 2; i < strings.size(); ++i)
	{
		new_topic.append(strings[i]);
		if (i != strings.size() - 1)
			new_topic.append(" ");
	}
	new_topic.append("\n");
	chan.set_topic(new_topic);
	std::string msg(":" + modifier->get_nickname() + "!~" + modifier->get_username() + "@" + modifier->get_hostname() + " TOPIC " + chan.get_name() + " " + new_topic + "\n");
	chan.send_to_members(msg);
}

/*******************************************************/
/* MODE STUFF        	                               */
/*******************************************************/
int server::change_user_mode(user *command_author, std::vector<std::string>& strings)
{
	user* to_promote = find_user(strings[3]);
	channel &chan = find_channel(strings[1]);
	if (!to_promote || !chan.member_exists(to_promote->get_nickname()))
	{
		std::string rpl_msg = rpl_string(command_author, ERR_NOSUCHNICK, "No such nick", strings[1], strings[3]);
		send(command_author->get_fd(), rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	if (!strings[2].compare("+o"))
	{
		chan.add_operator(to_promote);
		std::string msg(":" + command_author->get_nickname() + "!~" + command_author->get_username() + "@" + command_author->get_hostname() + " MODE " + chan.get_name() + " " + strings[2] + " " + to_promote->get_nickname() + "\n");
		chan.send_to_members(msg);
	}
	else
	{
		std::string rpl_msg = rpl_string(command_author, ERR_UMODEUNKNOWNFLAG, "Unknown MODE flag");
		send(command_author->get_fd(), rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
	return 0;
}
int server::change_mode(int userFd, std::vector<std::string>& strings)
{
	user *command_author = find_user(userFd);
	if (!channel_exists(strings[1]))
	{
		std::string rpl_msg = rpl_string(command_author, ERR_NOSUCHCHANNEL, "No such channel", strings[1]);
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1; //channel does not exists
	}
	channel &chan = find_channel(strings[1]);
	if (!chan.is_operator(command_author->get_nickname()) && strings.size() >= 3)
	{
		std::string rpl_msg = rpl_string(command_author, ERR_CHANOPRIVSNEEDED, "You're not channel operator", chan.get_name());
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1; //not a chan operator
	}
	if (strings.size() == 2)
	{
		std::string msg = prefix_user(command_author, RPL_CHANNELMODEIS) + " " + chan.get_name() + " " + chan.get_mode() + "\n";
		send(userFd, msg.data(), msg.length(), 0);
	}
	else if (strings.size() == 3)
	{
		if (set_chan_modes(chan, strings[2]))
		{
			std::string rpl_msg = rpl_string(command_author, ERR_UMODEUNKNOWNFLAG, "Unknown MODE flag");
			std::cout << "**" << rpl_msg << "**" << std::endl;
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			return -1;
		}
		std::string msg(":" + command_author->get_nickname() + "!~" + command_author->get_username() + "@" + command_author->get_hostname() + " MODE " + chan.get_name() + " " + strings[2] + "\n");
		chan.send_to_members(msg);
	}
	else if (strings.size() == 4)
		change_user_mode(find_user(userFd), strings);
	return 0;
}
int server::invitation(int userFd, std::vector<std::string>& strings)
{
	std::string chan_name = strings[2];
	std::string invited_user = strings[1];
	if (!channel_exists(strings[2]))
		return -1; //return rpl channel does not exists
	user *invited = find_user(invited_user);
	user *member = find_user(userFd);


	invited->add_invitation(chan_name);
	std::string invited_msg(":" + member->get_nickname() + "!~" + member->get_username() + "@" + member->get_hostname() + " INVITE " + invited->get_nickname() + " :" + chan_name + "\n");
	send(invited->get_fd(), invited_msg.data(), invited_msg.length(), 0);
	std::string member_msg(":" + member->get_nickname() + "!~" + member->get_username() + "@" + member->get_hostname() + RPL_INVITING + member->get_nickname() + " " + invited->get_nickname() + " " + chan_name + "\n");
	send(member->get_fd(), member_msg.data(), member_msg.length(), 0);
	return 0;
}
int	server::set_chan_modes(channel &chan, std::string modes)
{
	for (size_t i = 1; i < modes.size(); i++)
	{
		if (modes[i] != 'i' && modes[i] != 'k')
			return -1;
	}
	for (size_t i = 0; i < modes.size(); i++)
	{
		if (modes[0] == '+')
		{
			if (modes[i] == 'i')
				chan.mode[INVITE_ONLY_MODE] = true;
			else if (modes[i] == 'k')
				chan.mode[KEY_MODE] = true;
		}
		if (modes[0] == '-')
		{
			if (modes[i] == 'i')
				chan.mode[INVITE_ONLY_MODE] = false;
			else if (modes[i] == 'k')
				chan.mode[KEY_MODE] = false;
		}
	}
	return 0;
}

/*******************************************************/
/* QUIT STUFF        	                               */
/*******************************************************/
int server::quit(int userFd)
{
	user *user_to_quit = find_user(userFd);
	std::string msg(":" + user_to_quit->get_nickname() + "!~" + user_to_quit->get_username() + "@" + user_to_quit->get_hostname() + " QUIT :Client quit\n");
	for (std::vector<channel>::iterator chan = channels.begin(); chan != channels.end(); ++chan)
	{
		if (chan->member_exists(*user_to_quit))
		{
			chan->send_to_members(msg);
			chan->remove_member(chan->find_member(user_to_quit->get_fd()));
		}
	}
	this->remove_user(user_to_quit);
	return 0;
}


/*******************************************************/
/* PART STUFF        	                               */
/*******************************************************/
int server::part(int userFd, std::vector<std::string>& strings)
{
	user *leaver = find_user(userFd);
	std::vector<std::string> recipients = split_string(strings[1], ',');
	for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end(); ++it)
	{
		if (!channel_exists(*it))
		{
			std::string rpl_msg = rpl_string(leaver, ERR_NOSUCHCHANNEL, "No such channel", *it);
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			continue ;
		}
		channel& chan_recipient = find_channel(*it);
		if (!chan_recipient.member_exists(userFd))
		{
			std::string rpl_msg = rpl_string(leaver, ERR_NOTONCHANNEL, "You're not on that channel", *it);
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			continue ;
		}
		std::string msg(":" + leaver->get_nickname() + " PART " + *it + "\n");
		chan_recipient.send_to_members(msg);
		chan_recipient.remove_member(leaver);
		std::cout << "Part message -- > " << msg << std::endl;
	}
	return 0;
}

/*******************************************************/
/* LIST STUFF        	                               */
/*******************************************************/
int	server::list(int userFd, std::vector<std::string>& strings)
{
	// std::string users;
	// std::stringstream ss;
	std::string	msg;
	user	*lister = find_user(userFd);

	msg = rpl_string(lister, RPL_LISTSTART, "Channel: Users  Name");
	send(userFd, msg.data(), msg.length(), 0);
	if (strings.size() > 2)
	{
		msg = rpl_string(lister, RPL_NOTICE, "Invalid parameters for /LIST");
		send(userFd, msg.data(), msg.length(), 0);
	}
	else
	{
		if (strings.size() == 2 && !strings[1].empty())
		{
			std::vector<std::string>	chan_list = split_string(strings[1], ',');
			for (size_t i = 0; i < chan_list.size(); i++)
			{
				if (channel_exists(chan_list[i]))
				{
					channel	chan = find_channel(chan_list[i]);
					std::string users;
					std::stringstream ss;
					ss << chan.members.size();
					users = ss.str();
					msg = prefix_user(lister, RPL_LIST) + " " + chan.get_name() + " " + users;
					// if (!channels[i].get_topic().empty())
					//  	msg += " :" + "TOPIC HERE";
					// else
					msg += "\n";
				}
				else
					msg = rpl_string(lister, ERR_NOSUCHCHANNEL, "No such channel", chan_list[i]);
				send(userFd, msg.data(), msg.length(), 0);
			}
		}
		else
		{
			for (size_t i = 0; i < channels.size(); i++)
			{
				//NEED TO CHANGE TOPIC CHECK WHEN IMPLEMENTED (ideally a string topic on channel)
				std::string users;
				std::stringstream ss;
				ss << channels[i].members.size();
				users = ss.str();
				msg = prefix_user(lister, RPL_LIST) + " " + channels[i].get_name() + " " + users;
				// if (!channels[i].get_topic().empty())
				//  	msg += " :" + "TOPIC HERE";
				// else
				msg += "\n";
				// std::string msg = prefix_user(command_author, RPL_CHANNELMODEIS) + " " + chan.get_name() + " " + chan.get_mode() + "\n";
				send(userFd, msg.data(), msg.length(), 0);
			}
		}
	}
	msg = rpl_string(lister, RPL_LISTEND, "End of /LIST");
	send(userFd, msg.data(), msg.length(), 0);
	return 0;
}

/*******************************************************/
/* PONG STUFF        	                               */
/*******************************************************/
int	server::pong(int userFd, std::vector<std::string>& strings)
{
	if (strings.size() > 1 && strings[1].length()) {
		std::string msg(":server PONG server :" + strings[1] + "\n");
		send(userFd, msg.data(), msg.length(), 0);
		return 0;
	}
	return -1;
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
	if (!chan.is_operator(kicker->get_nickname()))
	{
		std::string rpl_msg = rpl_string(kicker, ERR_CHANOPRIVSNEEDED, "You are not channel operator", chan_name);
		send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
		return -1;
	}
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
	if (strings.size() >= 4)
		for (size_t i = 3; i < strings.size(); ++i)
			reason.append(strings[i] + " ");
	std::string msg(":" + find_user(userFd)->get_nickname() + " KICK " + chan_name + " " + nickname + " :" + reason + "\n");
	chan.send_to_members(msg);
	chan.remove_member(chan.find_member(nickname));
	return 0;
}


/*******************************************************/
/* CHANNEL STUFF                                       */
/*******************************************************/
int server::join_channel(int userFd, std::vector<std::string> &strings)
{
	std::vector<std::string> channels = split_string(strings[1], ',');
	user *user_to_add = find_user(userFd);

	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			std::string rpl_msg = rpl_string(user_to_add, ERR_NOSUCHCHANNEL, "No such channel", *it);
			send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
			return -1;
		}
		if (!channel_exists(*it))
		{
			create_channel(*it, "fake_key");
			find_channel(*it).add_operator(user_to_add);
		}
		if (!find_channel(*it).member_exists(user_to_add->get_nickname()))
		{
			if (find_channel(*it).mode[INVITE_ONLY_MODE] && !user_to_add->is_invited(*it))
			{
				std::string rpl_msg = rpl_string(user_to_add, ERR_INVITEONLYCHAN, "Cannot join channel (+i)", *it);
				send(userFd, rpl_msg.data(), rpl_msg.length(), 0);
				return -1;
			}
			find_channel(*it).add_member(user_to_add);
			if (send_join_rpl(*it, userFd) < 0)
				return -1;
		}
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
user* server::find_user(std::string nickname)
{
	user* ret = NULL;
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if ((*it)->get_nickname() == nickname)
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
std::string server::build_privmsg(int userFd, std::vector<std::string> strings, std::string recipient, std::string message_type)
{
	std::string msg(":" + find_user(userFd)->get_nickname() + "!~" + find_user(userFd)->get_username() + "@" + find_user(userFd)->get_hostname() + message_type + recipient);
	//i = 2 because 0and 1 are the command and the recipient
	for (size_t i = 2; i < strings.size(); i++)
	{
		msg += " ";
		msg += strings[i];
	}
	msg += "\n";
	return msg;
}
int server::send_message_to_channel(int userFd, std::string &recipient, std::string msg, message_type type)
{
	channel &chan = find_channel(recipient);
	if (!chan.member_exists(*(find_user(userFd))))
	{
		if (type == PRIVMSG)
		{	std::string rpl_message(rpl_string(find_user(userFd), ERR_CANNOTSENDTOCHAN, "Cannot send to channel", recipient));
			if (send(userFd, rpl_message.data(), rpl_message.length(), 0) < 0)
			{
				perror(" Send failed()");
				return -1;
			}
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
		std::string msg = build_privmsg(userFd, strings, *it, std::string(" PRIVMSG "));
		if (channel_exists(*it) && send_message_to_channel(userFd, *it, msg, PRIVMSG) < 0)
			return -1; //send failed, continue to next loop ?
		else if (user_exists(*it) && send_message_to_user(*it, msg) < 0)
			return -1;//send failed, continue to next loop ?
	}
	
	return 0;
}
int server::send_notice(int userFd, std::vector<std::string> &strings)
{
	std::vector<std::string> recipients = split_string(strings[1], ',');
	for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end(); it++)
	{
		std::string msg = build_privmsg(userFd, strings, *it, std::string(" NOTICE "));
		if (channel_exists(*it) && send_message_to_channel(userFd, *it, msg, NOTICE) < 0)
			return -1; //send failed, continue to next loop ?
		else if (user_exists(*it) && send_message_to_user(*it, msg) < 0)
			return -1;//send failed, continue to next loop ?
	}
	
	return 0;
}

int server::send_welcome(int userFd)
{
	user *new_user = find_user(userFd);
	if (!new_user->get_nickname().empty() && !new_user->get_username().empty())
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
/* REMOVE                       			           */
/*******************************************************/
int server::remove_user(user *user_to_remove)
{
	if (!this->user_exists(user_to_remove->get_fd()))
	{
		std::cout << "user does not exist" << std::endl;
		return 0;
	}
	for (std::vector<user*>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		if ((*it)->get_fd() == user_to_remove->get_fd())
		{
			delete (*it);
			this->users.erase(it);
			break;
		}
	}
	return 1;
}
/*******************************************************/
/* GETTERS                       			           */
/*******************************************************/
std::string server::get_ip() { return this->ip; }
std::vector<user*> &server::get_users() { return this->users; }
std::vector<channel> &server::get_channels() { return this->channels; }
const int					&server::get_port() const { return this->port; }
const std::string			&server::get_password() const { return this->password; }


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
