#include "string_maker.hpp"

std::string prefix_user(const user *client, const char* rpl)
{
	std::string ret(":" + client->get_hostname() + rpl + client->get_nickname());
	return ret;
}

std::string rpl_string(const user *client, const char *rpl, const char *reason, std::string param1, std::string param2)
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

std::vector<std::string>	split_string( std::string& str_to_split, char separator ) {

	std::vector<std::string> ret_vec;

	size_t start = 0;
	size_t end = 0;
	while ( end != std::string::npos ) {
		end = str_to_split.find_first_of(separator, start);
		std::string str = str_to_split.substr(start, end - start);
		ret_vec.push_back(str);
		start = end + 1;
	}	
	return ret_vec;
}

