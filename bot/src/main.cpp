#include "bot.hpp"

int main(int argc, char** argv)
{
	std::string					server;
	std::string					port;
	std::string 				password;
	std::string					nick;
	std::string					channel;
	std::string					buffer;
	std::vector<std::string>	cmd_list;
	bool						end_bot = false;

	if (argc != 4) {
		std::cerr << "Usage: ./ircbot <server> <port> <password>" << std::endl;
		exit(-1);
	}
	server = argv[1];
	port = argv[2];
	password = argv[3];
	channel = "#botroom";
	nick = "ircbot";

	Bot	ircbot;
	if (!ircbot.connect(server, port, password))
	{
		std::cerr << "Could not connect to server." << std::endl;
		exit(-1);
	}
	if (!ircbot.identify(nick))
	{
		std::cerr << "Could not register to the server." << std::endl;
		exit(-1);
	}
	if (!ircbot.join(channel))
	{
		std::cerr << "Could not join the channel." << std::endl;
		exit(-1);
	}
	while (end_bot == false)
	{
		buffer = ircbot.get_data();
		if (ircbot.get_last_error() != IRC_RECIEVE_ERROR)
		{
			if (ircbot.parser(buffer, &cmd_list))
			{
				if (cmd_list[0] == "QUIT")
				{
					ircbot.notice("IRCBot Quitting...", channel);
					end_bot == true;
				}
			}
			cmd_list.clear();
		}
	}
	ircbot.quit();
	return 0;
}