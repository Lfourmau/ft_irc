#include "channel.hpp"

bool channel::memberExists(user member)
{
	for (size_t i = 0; i < this->members.size(); i++)
	{
		if (member.getFd() == this->members[i].getFd())
			return true;
	}
	return false;
}

int channel::addMember(user member)
{
	if (memberExists(member))
	{
		std::cout << "user exists" << std::endl;
		return 1;
	}
	this->members.push_back(member);
	return 0;
}

void channel::printMembers()
{
	for (size_t i = 0; i < this->members.size(); i++)
		std::cout << "User in channel {" << this->name << "} = " << this->members[i].getFd() << std::endl;
}