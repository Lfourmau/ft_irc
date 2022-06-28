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

void		channel::set_mode(chan_mode mode_to_set) { this->mode = mode_to_set; }
chan_mode 	channel::get_mode() { return this->mode ;}
void channel::print_members()
{
	for (std::vector<user*>::iterator it = this->members.begin(); it != this->members.end(); ++it)
		std::cout << "User in channel {" << this->name << "} = " << (*it)->get_fd() << "---" << (*it)->get_nickname() << std::endl;
}
