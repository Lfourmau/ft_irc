#include "server.hpp"

int server::parsing(char *input, int userFd)
{
	std::string toparse(input);
	if (toparse[0] != '/')
		send_message(toparse, userFd);
	std::vector<std::string> strings;
    std::istringstream stream(toparse);
    std::string word;    
    while (getline(stream, word, ' '))
        strings.push_back(word);
	if (!strings[0].compare("/join"))
		join_channel(strings[1], strings[2], userFd);
	printChannels();
	return 0;
}

int server::join_channel(std::string name, std::string key, int userFd)
{
	if (channelExists(name))
		findChannel(name).addMember(findUser(userFd));
	else
		createChannel(name, key, userFd);
	findUser(userFd).currentChan = &findChannel(name);
	findChannel(name).printMembers();
	return 0;
}

void server::createChannel(std::string name, std::string key, int userFd)
{
	channel toCreate(name, key);
	toCreate.addMember(findUser(userFd));
	channels.push_back(toCreate);
}



int server::addUser(int fd)
{
	if (userExists(fd))
		std::cout << "user already connected" << std::endl;
	else
	{
		user toAdd(fd);
		this->users.push_back(toAdd);
	}
	printUsers();
	return 0;
}

int server::send_message(std::string msg, int userFd)
{
	for (size_t i = 0; i < findUser(userFd).currentChan->members.size(); i++)
	{
		if (send(findUser(userFd).currentChan->members[i].getFd(), &msg, msg.length(), 0) < 0)
		{
			perror("  send() failed");
			break;
		}
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