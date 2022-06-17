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
#include <arpa/inet.h>
#include "user.hpp"

class user;

class channel
{
	public:
		channel(std::string id, std::string password = NULL) : name(id), key(password) {};
		channel() : name("NULL") {};
		~channel() {};
		std::string const 	&get_name() const {return name;};
		std::string const 	&get_key() const {return key;};
		bool 				member_exists(user member);
		int					add_member(user member);
		void 				print_members();
		std::vector<user> 	members;

	private:
		std::string			name;
		std::string			key;
		//mode 		chan_mode
};

#endif