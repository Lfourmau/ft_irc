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
#include "user.hpp"
class channel
{
	public:
		channel(std::string id, std::string password = NULL) : name(id), key(password) {};
		channel() : name("NULL") {};
		~channel() {};
		std::string const &getName() const {return name;};
		std::string const &getKey() const {return key;};
		bool 		memberExists(user member);
		int			addMember(user member);
		void 		printMembers();
	private:
		std::string			name;
		std::string			key;
		//int 		mode;
		std::vector<user> 	members;
};

#endif