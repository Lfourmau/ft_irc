#ifndef SERVER_HPP
# define SERVER_HPP

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
#include <sstream>
#include "channel.hpp"
#include "user.hpp"
#include "rpl.hpp"

enum mode
{
	INVITE_ONLY,
	OTHER_MODE,
};

class user;
class channel;

class server
{
	public:
		server(std::string key = NULL) : password(key), ip("10.1.8.2") {};
		~server() {};

		int 		parsing(std::string toparse, int userFd);
		void 		create_channel(std::string name, std::string key, int userFd);
		bool 		channel_exists(std::string chan);
		int 		join_channel(int userFd, std::string name, std::string key);
		bool 		user_exists(int fd);
		int			add_user(int fd, sockaddr_in &addr);
		channel 	&find_channel(std::string name);
		user 		&find_user(int userFd);
		int 		send_message(int userFd, std::vector<std::string> &cmd);
		int			send_welcome(int userFd);
		int			send_join_alert(std::string msg, std::string name);
		std::string get_ip();
		void		print_channels();
		void		print_users();

	private:
		//int port;
		std::string password;
		std::string ip;
		//int connected_users;
		std::vector<channel> channels;
		std::vector<user> users;
};

#endif