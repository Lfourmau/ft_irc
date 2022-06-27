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
#include "string_maker.hpp"

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
		~server();

		int 				parsing(std::string toparse, int userFd);
		void 				create_channel(std::string name, std::string key);
		bool 				channel_exists(std::string chan);
		int 				join_channel(int userFd, std::vector<std::string> &strings);
		bool 				user_exists(int fd);
		bool				user_exists(std::string name);
		int					add_user(int fd, sockaddr_in &addr);
		channel 			&find_channel(std::string name);
		user 				*find_user(int userFd);
		int					no_recipient_or_text(int userFd, std::vector<std::string> strings);
		int 				send_privmsg(int userFd, std::vector<std::string> &cmd);
		int 				send_message_to_channel(int userFd, std::string &recipient, std::string msg);
		int 				send_message_to_user(std::string &recipient, std::string msg);
		std::string 		build_privmsg(int userfd, std::vector<std::string> strings, std::string recipient);
		int					send_welcome(int userFd);
		int					send_join_notif(std::string msg, std::string name);
		int 				send_join_rpl(std::string channel_name, int userFd);
		std::string 		get_ip();
		void				print_channels();
		void				print_users();
		std::vector<user*> 	&get_users();
		std::vector<channel> 	&get_channels();
		std::vector<std::string>	parsing_join_input(std::string& strings);

	private:
		//int port;
		std::string password;
		std::string ip;
		//int connected_users;
		std::vector<channel> channels;
		std::vector<user*> users;
};

#endif
