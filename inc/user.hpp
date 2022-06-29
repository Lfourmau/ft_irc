#ifndef USER_HPP
# define USER_HPP

#include "server.hpp"
#include "channel.hpp"

#define MAX_NICK_LENGTH 12
#define INVALID_INCHARS " ,*?!@.:"
#define INVALID_STARTCHARS "$#"

class channel;
class server;

enum user_mode
{
	INVISIBLE_MODE,
	OPER_MODE,
	LOCAL_OPER_MODE,
	REGISTERED_MODE,
	WALLOPS_MODE,
};

class user
{
	public:
		user(int sock) : is_connected(0), invited(), fd(sock), command(), username("*"),realname("*"), nickname("*") { memset(buff, 0, 80); };
		~user() {};

		channel *currentChan;
		char buff[80];

		int	const 	&get_fd() const {return fd;};
		std::string get_command() { return command; };
		std::string get_username() const;
		std::string get_realname() const;
		std::string get_nickname() const;
		std::string get_hostname() const;
		user_mode 	get_mode() const;
		void	 	get_mode(user_mode) const;
		bool 		is_invited(std::string chan_name);
		void 		add_invitation(std::string chan_name);
		int 		set_command(char *buff);
		int 		set_nickname(std::vector<std::string> &strings, server& server);
		void 		set_hostname(sockaddr_in &addr);
		void 		set_mode(user_mode mode_to_set);
		int			my_register(std::vector<std::string> &strings);
		int			send_nickname_notif(std::string msg, server &server);
		int			check_nickname_validity(std::vector<std::string> &strings, server& server, std::string nick);

		int 						is_connected;
		std::vector<std::string> 	invited;

	private:
		int fd;
		user_mode mode;
		std::string command;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string nickname;
		std::string servername;
};


#endif
