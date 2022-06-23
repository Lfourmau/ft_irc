#include "channel.hpp"

bool channel::member_exists(user member)
{
	bool ret = false;
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if (member.get_fd() == (*it)->get_fd())
			ret = true;
	}
	return ret;
}

int channel::add_member(user *member)
{
	if (member_exists(*member))
	{
		std::cout << "user exists" << std::endl;
		return 1;
	}
	this->members.push_back(member);
	return 0;
}

void channel::print_members()
{
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
		std::cout << "User in channel {" << this->name << "} = " << (*it)->get_fd() << "---" << (*it)->get_nickname() << std::endl;
}