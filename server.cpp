#include "server.hpp"

int server::parsing(std::string toparse)
{
	std::vector<std::string> strings;
    std::istringstream stream(toparse);
    std::string word;    
    while (getline(stream, word, ' '))
        strings.push_back(word);

	if (!strings[0].compare("/join"))
		join_channel(strings[1], strings[2]);
	return 0;
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

void server::createChannel(std::string name, std::string key)
{
	channel toCreate(name, key);
	channels.push_back(toCreate);
	printChannels();
}

int server::join_channel(std::string name, std::string key)
{
	if (channelExists(name))
		std::cout << "channel exists" << std::endl;
	else
		createChannel(name, key);
	return 0;
}




//UTILS
void server::printChannels()
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::cout << channels[i].getName() << std::endl;
	}
	
}