#ifndef STRING_MAKER_HPP
# define STRING_MAKER_HPP

#include "server.hpp"

std::string prefix_user(const user &client, const char* rpl);
std::string rpl_string(const user &client, const char *rpl, const char *reason, std::string param1 = "", std::string param2 = "");

#endif