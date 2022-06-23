#ifndef USER_HPP
# define USER_HPP

#include "server.hpp"
#include "channel.hpp"

#define MAX_NICK_LENGTH 12
#define INVALID_INCHARS " ,*?!@.:"
#define INVALID_STARTCHARS "$#"

class channel;
class server;

class user
{
	public:
		user(int sock) : is_connected(0), fd(sock), command(), nickname("*") { memset(buff, 0, 80); };
		~user() {};

		channel *currentChan;
		char buff[80];

		int	const 	&get_fd() const {return fd;};
		std::string get_command() { return command; };
		std::string get_username() const;
		std::string get_realname() const;
		std::string get_nickname() const;
		std::string get_hostname() const;
	
		int 		set_command(char *buff);
		int 		set_nickname(std::vector<std::string> &strings, server& server);
		void 		set_hostname(sockaddr_in &addr);
		int			my_register(std::vector<std::string> &strings);
		int			send_nickname_notif(std::string msg, server &server);

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

std::ostream& operator<<(std::ostream &os, const user& client) {
	os << client.get_nickname();
	return (os);
};

#endif