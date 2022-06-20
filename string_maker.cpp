#include "string_maker.hpp"

std::string prefix_user(const user &client, const char* rpl)
{
	std::string ret(":" + client.get_hostname() + rpl + client.get_nickname());
	return ret;
}

std::string rpl_string(const user &client, const char *rpl, const char *reason, std::string param1, std::string param2)
{
	std::string ret;

	if (param2.size())
		ret = prefix_user(client, rpl) + " " + param1 + " " + param2 + " :" + reason + "\n";
	else if (param1.size())
		ret = prefix_user(client, rpl) + " " + param1 + " :" + reason + "\n";
	else
		ret = prefix_user(client, rpl) + " :" + reason + "\n";
	return (ret);
}