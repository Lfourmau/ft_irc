#include "bot.hpp"

void	run(int socketFd, const std::string& password = "")
{
	int			len;
	char		buffer[MAX_BUFFER_LENGTH];
	std::string	content;
	std::string	line;

	std::string	msg = "";
	if (password.length())
		msg = "PASS " + password + "\r\n";
	msg += "NICK ";
	msg += NICK;
	msg += "\r\n";
	msg += "USER ";
	msg += NAME;
	msg += " 0 * ";
	msg += REALNAME;
	msg += "\r\n";
	send(socketFd, msg.data(), msg.length(), 0);
	sleep(2);
	msg = "";
	msg += "JOIN ";
	msg += CHANNEL;
	msg += "\r\n";
	send(socketFd, msg.data(), msg.length(), 0);

	while ((len = recv(socketFd, buffer, MAX_BUFFER_LENGTH - 1, 0)) > 0)
	{
		buffer[len] = 0;
		content += buffer;
		if (content.find('\n') != std::string::npos) {
			content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

			std::istringstream ss(content.c_str());
			while (std::getline(ss, line, '\n')) {
				if (ss.eof() && !line.empty())
					content = line;
				else
					content.clear();
				std::string command = "PRIVMSG #bot ";
				if (line.find(command) != std::string::npos) {
					line.erase(0, line.find(command) + command.length());
					std::string message = command + ":";
					if (line.find("Hello") != std::string::npos)
						message += "Hi ! Happy to see you here !";
					else if (line.find("Bye") != std::string::npos)
						message += "I am very sad to see you leave";
					else if (line.find("What's up ?") != std::string::npos)
						message += "I will feel good if you pass my IRC";
					else
						message += "Send 'Hello', 'Bye' or 'What's up ?'";
					message += "\r\n";
					send(socketFd, message.c_str(), message.length(), 0);
				}
			}
		}
	}
}

int	main(int argc, char** argv)
{
	int					socketFd;
	int					port;
	struct sockaddr_in	serv_addr;
	struct hostent		*server;
	char				*pEnd;

	if (argc < 3 || argc > 4)
	{
		std::cerr << "Usage: ./ircbot <server> <port> [<password>]" << std::endl;
		exit(-1);
	}
	std::cout << "Bot loading..." << std::endl;
	port = std::strtol(argv[2], &pEnd, 10);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1)
	{
		std::cerr << "Something went wrong when initializing socket data." << std::endl;
		exit(-1);
	}
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		std::cerr << "Something went wrong when initializing server data." << std::endl;
		exit(-1);
	}
	std::memcpy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	if (connect(socketFd, (struct  sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cerr << "Could not connect to the server." << std::endl;
		exit(-1);
	}
	if (argc == 4)
		run(socketFd, argv[3]);
	else
		run(socketFd);
	close(socketFd);
	shutdown(socketFd, SHUT_RDWR);	
	return 0;
}