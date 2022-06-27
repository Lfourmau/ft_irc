#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "server.hpp"
#include "user.hpp"

class user;

enum chan_mode
{
	ban,
	exception,
	client_limit,
	invite_only,
	invite_exception,
	key,
	moderated,
	secret,
	protected_topic,
	no_external_messages,

};

class channel
{
	public:
		channel(std::string id, std::string password = NULL) : name(id), key(password) {};
		channel() : name("NULL") {};
		~channel() {};
		std::string const 	&get_name() const {return name;};
		std::string const 	&get_key() const {return key;};
		bool 				member_exists(user member);
		bool 				member_exists(std::string nickname);
		int					add_member(user *member);
		int					remove_member(user *member);
		user 				*find_member(std::string nickname);
		int					send_to_members(std::string msg);
		void 				print_members();
		chan_mode 			get_mode();
		void 				set_mode(chan_mode);
		std::vector<user*> 	members;

	private:
		std::string			name;
		std::string			key;
		chan_mode 			mode;
};

#endif