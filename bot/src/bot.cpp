#include "bot.hpp"

Bot::Bot() {}

Bot::~Bot() {}

//attempts to connect to the server
bool		Bot::connect(std::string server, std::string port, std::string password)
{

}

//attempts to register with a given nick; name and real name will be built-in
bool		Bot::identify(std::string nick)
{

}

//attempts to join a given channel
bool		Bot::join(std::string channel)
{

}

//retrieves messages sent by the server and puts them in a string buffer
std::string	Bot::get_data()
{

}

//checks if the message recieved is an RPL or an error message? returns an ID of some sort
int			Bot::get_last_error()
{

}

//checks if first character matches command prefix (returns true) and splits the message
//into parameters (first param is the command) else returns false
bool		Bot::parser(std::string buffer, std::vector<std::string> *cmd_list)
{

}

//sends a notice to the channel
void		Bot::notice(std::string message, std::string channel)
{

}

//leaves the server
void		Bot::quit()
{

}