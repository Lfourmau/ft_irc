#include "server.hpp"

int server::parsing(std::string toparse, int userFd)
{
	size_t sep = toparse.find("\r\n");

	while (sep != std::string::npos)
	{
		std::string cmd(toparse.begin(), toparse.begin() + sep);
		std::cout << "CMD = " << cmd << "**" << std::endl;
		std::vector<std::string> strings;
		std::istringstream stream(cmd);
		std::string word;
		while (getline(stream, word, ' '))
			strings.push_back(word);
		if (!strings[0].compare("JOIN"))
			join_channel(userFd, strings[1], strings[2]);
		else if (!strings[0].compare("NICK"))
			findUser(userFd).setNickname(strings[1]);
		else if (!strings[0].compare("USER"))
			findUser(userFd).my_register(strings);
		//else if (findUser(userFd).getCommand().find('\n', 0) != std::string::npos)
		//	send_message(cmd, userFd);
		toparse.erase(toparse.begin(), toparse.begin() + sep + 2);
		sep = toparse.find("\r\n", sep + 1);
		std::cout << "TOPARSE == " << toparse << "**" << std::endl;
	}
	if (!findUser(userFd).getNickname().empty() && !findUser(userFd).getUsername().empty() && findUser(userFd).is_connected == 0)
	{
		//std::string end("CAP END\n");
		//send(userFd, end.data(), end.length(), 0);
		std::string welcome(":10.1.8.2 001 " + findUser(userFd).getNickname() + " :Welcome to the Internet Relay Network " + findUser(userFd).getNickname() + "\n");
		std::cout << "Welcome sent --> [" << welcome.data() << "]" << std::endl;
		send(userFd, welcome.data(), welcome.length(), 0);
		findUser(userFd).is_connected = 1;
	}
	printChannels();
	return 0;
}

int server::join_channel(int userFd, std::string name, std::string key)
{
	if (channelExists(name))
		findChannel(name).addMember(findUser(userFd));
	else
		createChannel(name, key, userFd);
	findUser(userFd).currentChan = &findChannel(name);
	std::string msg(":" + findUser(userFd).getNickname() + " JOIN " + name + "\n");
	std::cout << "MSG --> " << msg << "**" << std::endl;
	send(userFd, msg.data(), msg.length(), 0);
	findChannel(name).printMembers();
	return 0;
}

void server::createChannel(std::string name, std::string key, int userFd)
{
	channel toCreate(name, key);
	toCreate.addMember(findUser(userFd));
	channels.push_back(toCreate);
}



int server::addUser(int fd, sockaddr_in &addr)
{
	if (userExists(fd))
		std::cout << "user already connected" << std::endl;
	else
	{
		user toAdd(fd);
		toAdd.setHostname(addr);
		this->users.push_back(toAdd);
	}
	printUsers();
	return 0;
}

int server::send_message(std::string msg, int userFd)
{
	char nl = '\n';
	char *nealine = &nl;
	for (size_t i = 0; i < findUser(userFd).currentChan->members.size(); i++)
	{
		if (send(findUser(userFd).currentChan->members[i].getFd(), msg.data(), msg.length(), 0) < 0)
		{
			perror("  send() failed");
			break;
		}
		send(findUser(userFd).currentChan->members[i].getFd(), nealine, 1, 0);
	}
	return 0;
}

//EXISTS
bool server::userExists(int fd)
{
	for (size_t i = 0; i < this->users.size(); i++)
	{
		if (fd == this->users[i].getFd())
			return true;
	}
	return false;
}
bool server::channelExists(std::string chan)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (chan == this->channels[i].getName())
			return true;
	}
	return false;
	
}

//FINDS
channel& server::findChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (channels[i].getName() == name)
			return channels[i];
	}
	//don't know how return because if i call this function, the channel exists
	return (channels[0]);
}
user& server::findUser(int userFd)
{
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[i].getFd() == userFd)
			return users[i];
	}
	//don't know how return because if i call this function, the user exists
	return (users[0]);
}


//UTILS
void server::printChannels()
{
	for (size_t i = 0; i < channels.size(); i++)
		std::cout << "Channel--> " << channels[i].getName() << std::endl;
	
}
void server::printUsers()
{
	for (size_t i = 0; i < users.size(); i++)
		std::cout << "User--> " << users[i].getFd() << std::endl;
	
}