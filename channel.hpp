#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "server.hpp"
#include "user.hpp"

class user;

enum chan_mode
{
	BAN_MODE,
	EXCPTION_MODE,
	CLIENT_LIMIT_MODE,
	INVITE_ONLY_MODE,
	INVITE_EXCEPTION_MODE,
	KEY_MODE,
	MODERATED_MODE,
	SECRET_MODE,
	PROTECTED_TOPIC_MODE,
	NO_EXTERNAL_MESSAGES_MODE,

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
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
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