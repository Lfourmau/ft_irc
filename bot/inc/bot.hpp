#include <string>
#include <iostream>
#include <vector>
// #include <sstream>

#define IRC_RECIEVE_ERROR 1//tochange
#define	PREFIX !

class Bot
{
public:
	Bot();
	~Bot();

	bool		connect(std::string server, std::string port, std::string password);
	bool		identify(std::string nick);
	bool		join(std::string channel);
	std::string	get_data();
	int			get_last_error();
	bool		parser(std::string buffer, std::vector<std::string> *cmd_list);
	void		notice(std::string message, std::string channel);
	void		quit();
};