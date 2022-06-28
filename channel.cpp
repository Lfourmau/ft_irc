#include "channel.hpp"

std::string	channel::get_mode() const
{
	std::string	ret = "+";

	if (mode[INVITE_ONLY_MODE])
		ret += "i";
	if (mode[KEY_MODE])
		ret += "k";
	return ret;
}

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
bool channel::member_exists(std::string nickname)
{
	bool ret = false;
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if (nickname == (*it)->get_nickname())
			ret = true;
	}
	return ret;
}

user *channel::find_member(std::string nickname)
{
	user* ret = NULL;
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if ((*it)->get_nickname() == nickname)
		{
			ret = (*it);
			break;
		}
	}
	return (ret);
}

user *channel::find_member(int userFd)
{
	user* ret = NULL;
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if ((*it)->get_fd() == userFd)
		{
			ret = (*it);
			break;
		}
	}
	return (ret);
}

int channel::add_operator(user *member)
{
	if (is_operator(member->get_nickname()))
	{
		std::cout << "user exists" << std::endl;
		return 1;
	}
	this->operators.push_back(member);
	return 0;
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
int channel::remove_member(user *member)
{
	if (!member_exists(*member))
	{
		std::cout << "user does not exist" << std::endl;
		return 0;
	}
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if ((*it)->get_nickname() == member->get_nickname())
		{
			this->members.erase(it);
			break;
		}
	}
	return 1;
}

int	channel::send_to_members(std::string msg)
{
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
	{
		if (send((*it)->get_fd(), msg.data(), msg.length(), 0) < 0)
		{
			perror(" send() failed");
			continue ;
		}
	}
	return 0;
}
void channel::print_members()
{
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
		std::cout << "User in channel {" << this->name << "} = " << (*it)->get_fd() << "---" << (*it)->get_nickname() << std::endl;
}

bool		channel::is_operator(std::string nickname)
{
	for (std::vector<user*>::iterator it = this->operators.begin(); it != this->operators.end(); ++it)
	{
		if ((*it)->get_nickname() == nickname)
			return true;
	}
	return false;
}