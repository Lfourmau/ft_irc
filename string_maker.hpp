#ifndef STRING_MAKER_HPP
# define STRING_MAKER_HPP

#include "server.hpp"

# define PORT_NUMBER_INF		(500)
# define PORT_NUMBER_SUP		(65535)
# define INVALID_PORT_NUMBER	(-1)

std::string prefix_user(const user *client, const char* rpl);
std::string rpl_string(const user *client, const char *rpl, const char *reason, std::string param1 = "", std::string param2 = "");

std::vector<std::string>	split_string( std::string& str_to_split, char separator ); 

int		parse_port(std::string str);

#endif
