#include "channel.hpp"

bool channel::member_exists(user member)
{
	for (size_t i = 0; i < this->members.size(); i++)
	{
		if (member.get_fd() == this->members[i].get_fd())
			return true;
	}
	return false;
}

int channel::add_member(user member)
{
	if (member_exists(member))
	{
		std::cout << "user exists" << std::endl;
		return 1;
	}
	this->members.push_back(member);
	return 0;
}

void channel::print_members()
{
	for (size_t i = 0; i < this->members.size(); i++)
		std::cout << "User in channel {" << this->name << "} = " << this->members[i].get_fd() << std::endl;
}