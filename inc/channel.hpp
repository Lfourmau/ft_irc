#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "server.hpp"
#include "user.hpp"

class user;

enum chan_mode
{
	INVITE_ONLY_MODE,
	KEY_MODE,
};

class channel
{
	public:
		channel(std::string id, std::string password = NULL) : name(id), key(password), topic("") {};
		channel() : name("NULL") {};
		~channel() {};
		std::string const 	&get_name() const {return name;};
		std::string const 	&get_key() const {return key;};
		std::string			get_mode() const;
		bool 				member_exists(user member);
		bool 				member_exists(std::string nickname);
		int					add_member(user *member);
		int					remove_member(user *member);
		user 				*find_member(std::string nickname);
		user 				*find_member(int userFd);
		int					send_to_members(std::string msg);
		bool				is_operator(std::string nickname);
		int 				add_operator(user *member);
		void 				print_members();
		void 				set_topic(std::string new_topic);
		std::string			&get_topic();
		std::vector<user*> 	members;
		std::vector<user*> 	operators;
		bool 				mode[2];

	private:
		std::string			name;
		std::string			key;
		std::string 		topic;
};

#endif
