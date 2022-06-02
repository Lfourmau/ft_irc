#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <stdio.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>

class channel
{
	public:
		channel(std::string id, std::string password = NULL) : name(id), key(password) {};
		~channel() {};
		std::string getName() {return name;};
		std::string getKey() {return key;};

	private:
		std::string name;
		std::string key;
		//int 		mode;
};

#endif