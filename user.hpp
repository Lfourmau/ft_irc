#ifndef USER_HPP
# define USER_HPP

#include "channel.hpp"
#include "server.hpp"
class channel;

class user
{
	public:
		user(int sock) : is_connected(0), fd(sock), command(), nickname() { memset(buff, 0, 80); };
		~user() {};

		channel *currentChan;
		char buff[80];

		int	const 	&get_fd() const {return fd;};
		std::string get_command() { return command; };
		std::string get_username();
		std::string get_realname();
		std::string get_nickname();
		std::string get_hostname();
	
		void 		set_command(char *buff);
		int 		set_nickname(std::string nick);
		int			init_nickname(std::string nick);
		void 		set_hostname(sockaddr_in &addr);
		int			my_register(std::vector<std::string> &strings);
		int 		is_connected;

	private:
		int fd;
		std::string command;
		std::string username;
		std::string realname;
		std::string hostname;
		std::string nickname;
		std::string servername;
};

#endif