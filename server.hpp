#ifndef SERVER_HPP
# define SERVER_HPP

# define QUIT (-1)
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

class user;
class channel;

enum message_type
{
	PRIVMSG,
	NOTICE,
};

class server
{
	public:
		server(int port, std::string key );
		~server();

		int 				parsing(std::string toparse, int userFd);
		void 				create_channel(std::string name, std::string key);
		bool 				channel_exists(std::string chan);
		int 				join_channel(int userFd, std::vector<std::string> &strings);
		int 				kick(int userFd, std::vector<std::string>& strings);
		int 				change_mode(int userFd, std::vector<std::string>& strings);
		int 				change_user_mode(user *client, std::vector<std::string>& strings);
		int 				invitation(int userFd, std::vector<std::string>& strings);
		int 				set_chan_modes(channel &chan, std::string modes);
		int 				part(int userFd, std::vector<std::string>& strings);
		int					pong(int userFd, std::vector<std::string>& strings);
		int 				topic(int userFd, std::vector<std::string>& strings);
		void				set_topic(int userFd, channel &chan, std::vector<std::string> strings);
		int 				quit(int userFd);
		bool 				user_exists(int fd);
		bool				user_exists(std::string name);
		int					add_user(int fd, sockaddr_in &addr);
		channel 			&find_channel(std::string name);
		user 				*find_user(int userFd);
		user 				*find_user(std::string nickname);
		int					no_recipient_or_text(int userFd, std::vector<std::string> strings);
		int 				send_privmsg(int userFd, std::vector<std::string> &cmd);
		int 				send_message_to_channel(int userFd, std::string &recipient, std::string msg, message_type type);
		int 				send_message_to_user(std::string &recipient, std::string msg);
		std::string 		build_privmsg(int userfd, std::vector<std::string> strings, std::string recipient, std::string message_type);
		int					send_welcome(int userFd);
		int 				send_notice(int userFd, std::vector<std::string> &strings);
		int					send_join_notif(std::string msg, std::string name);
		int 				send_join_rpl(std::string channel_name, int userFd);
		int 				fin_and_send_kick_rpl(int userFd, std::string chan_name, std::string nickname);
		std::string 		get_ip();
		int 				remove_user(user *user_to_remove);
		void				print_channels();
		void				print_users();
		std::vector<user*> 	&get_users();
		std::vector<channel> 	&get_channels();
		std::vector<std::string>	parsing_join_input(std::string& strings);
		const int					&get_port() const;
		const std::string			&get_password() const;

	private:
		int 					port;
		std::string 			password;
		std::string 			ip;
		//int connected_users;
		std::vector<channel> 	channels;
		std::vector<user*>		users;
};

#endif
