#include "channel.hpp"
#include "server.hpp"

#include <fcntl.h>

int main ( int argc, char **argv )
{
	int    len, rc, on = 1;
	int    listen_sd = -1, new_sd = -1;
	bool   end_server = false, compress_array = false;
	int    close_conn;
	struct sockaddr_in6   addr;
	std::vector<struct pollfd> fds;
	int    timeout;

	if (argc != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		exit(-1);
	}

	int server_port = parse_port(std::string(argv[1]));
	if (server_port == INVALID_PORT_NUMBER) {
		std::cerr << "Invalid port number" << std::endl;
		exit(-1);
	}

	server my_serv(server_port, std::string(argv[2]));

	/*************************************************************/
	/* Create an AF_INET6 stream socket to receive incoming      */
	/* connections on                                            */
	/*************************************************************/
	listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
	if (listen_sd < 0)
	{
		perror("socket() failed");
		exit(-1);
	}

	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
	if (rc < 0)
	{
		perror("setsockopt() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for      */
	/* the incoming connections will also be nonblocking since   */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	rc = fcntl(listen_sd, F_SETFL, O_NONBLOCK);
	if (rc < 0)
	{
		perror("fcntl() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	addr.sin6_len = sizeof(addr);
	addr.sin6_family = AF_INET6;
	addr.sin6_flowinfo = 0;
	addr.sin6_port = htons(my_serv.get_port());
	addr.sin6_addr = in6addr_any; //global variable
	rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));

	if (rc < 0)
	{
		perror("bind() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	rc = listen(listen_sd, 32);
	if (rc < 0)
	{
		perror("listen() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Initialize the pollfd structure                           */
	/*************************************************************/
	memset(&fds, 0 , sizeof(fds));

	/*************************************************************/
	/* Set up the initial listening socket                        */
	/*************************************************************/
	struct pollfd initial_listening;

	initial_listening.fd = listen_sd;
	initial_listening.events = POLLIN;
	fds.push_back(initial_listening);
	/*************************************************************/
	/* Initialize the timeout to 3 minutes. If no                */
	/* activity after 3 minutes this program will end.           */
	/* timeout value is based on milliseconds.                   */
	/*************************************************************/
	timeout = (3 * 60 * 1000);

	/*************************************************************/
	/* Loop waiting for incoming connects or for incoming data   */
	/* on any of the connected sockets.                          */
	/*************************************************************/
	while (end_server == false)
	{
		/***********************************************************/
		/* Call poll() and wait 3 minutes for it to complete.      */
		/***********************************************************/
		std::cout << "Waiting on poll()... listening on port ";
		std::cout << my_serv.get_port() << ", with key " << my_serv.get_password() << std::endl;

		rc = poll(&fds[0], fds.size(), timeout);

		/***********************************************************/
		/* Check to see if the poll call failed.                   */
		/***********************************************************/
		if (rc < 0)
		{
			perror("  poll() failed");
			break;
		}

		/***********************************************************/
		/* Check to see if the 3 minute time out expired.          */
		/***********************************************************/
		if (rc == 0)
		{
			std::cout << "  poll() timed out.  End program." << std::endl;
			break;
		}

		/***********************************************************/
		/* One or more descriptors are readable.  Need to          */
		/* determine which ones they are.                          */
		/***********************************************************/
		for (size_t i = 0; i < fds.size(); i++)
		{
			/*********************************************************/
			/* Loop through to find the descriptors that returned    */
			/* POLLIN and determine whether it's the listening       */
			/* or the active connection.                             */
			/*********************************************************/
			if(fds[i].revents == 0)
				continue;

			/*********************************************************/
			/* If revents is not POLLIN, it's an unexpected result,  */
			/* log and end the server.                               */
			/*********************************************************/
			// when a connection is closed, revents == 17
			if(fds[i].revents != POLLIN && fds[i].revents != 17) 
			{
				std::cout << "  Error! revents = " << fds[i].revents << std::endl;
				end_server = true;
				break;
			}
			if (fds[i].fd == listen_sd)
			{
				/*******************************************************/
				/* Listening descriptor is readable.                   */
				/*******************************************************/
				std::cout << "  Listening socket is readable" << std::endl;

				/*******************************************************/
				/* Accept all incoming connections that are            */
				/* queued up on the listening socket before we         */
				/* loop back and call poll again.                      */
				/*******************************************************/
				
				struct sockaddr_in   client_addr;
			
				client_addr.sin_family = AF_INET6;
				socklen_t addr_len = sizeof(client_addr.sin_addr);
				do
				{
					/*****************************************************/
					/* Accept each incoming connection. If               */
					/* accept fails with EWOULDBLOCK, then we            */
					/* have accepted all of them. Any other              */
					/* failure on accept will cause us to end the        */
					/* server.                                           */
					/*****************************************************/
					new_sd = accept(listen_sd, (sockaddr *)&client_addr.sin_addr, &addr_len);
					if (new_sd < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							perror("  accept() failed");
							end_server = true;
						}
						break;
					}

					/*****************************************************/
					/* Add the new incoming connection to the            */
					/* pollfd structure                                  */
					/*****************************************************/
					std::cout << " New incoming connection - " << new_sd << std::endl;
					struct pollfd add_connect;
					add_connect.fd = new_sd;
					add_connect.events = POLLIN;
					fds.push_back(add_connect);
					my_serv.add_user(new_sd, client_addr);

					/*****************************************************/
					/* Loop back up and accept another incoming          */
					/* connection                                        */
					/*****************************************************/
				} while (new_sd != -1);
			}

			/*********************************************************/
			/* This is not the listening socket, therefore an        */
			/* existing connection must be readable                  */
			/*********************************************************/

			else
			{
				std::cout << "   Descriptor " << fds[i].fd << " is readable" << std::endl;
				close_conn = false;
				/*******************************************************/
				/* Receive all incoming data on this socket            */
				/* before we loop back and call poll again.            */
				/*******************************************************/

				while (true)
				{

					/*****************************************************/
					/* Receive data on this connection until the         */
					/* recv fails with EWOULDBLOCK. If any other         */
					/* failure occurs, we will close the                 */
					/* connection.                                       */
					/*****************************************************/
					memset(my_serv.find_user(fds[i].fd)->buff, 0, 80);
					rc = recv(fds[i].fd, my_serv.find_user(fds[i].fd)->buff, sizeof(my_serv.find_user(fds[i].fd)->buff), 0);
					std::cout << "{" << my_serv.find_user(fds[i].fd)->buff << "}" << std::endl;
					if (rc < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							perror("  recv() failed");
							close_conn = true;
						}
						break;
					}

					/*****************************************************/
					/* Check to see if the connection has been           */
					/* closed by the client                              */
					/*****************************************************/
					if (rc == 0)
					{
						std::cout << "  Connection closed" << std::endl;
						my_serv.quit(fds[i].fd);
						close_conn = true;
						break;
					}

					/*****************************************************/
					/* Data was received                                 */
					/* Parsing changes                                   */
					/*****************************************************/
					len = rc;
					std::cout << "  " << len << " bytes received" << std::endl;
					//parse instead of echo data to the client
					if (my_serv.find_user(fds[i].fd)->set_command(my_serv.find_user(fds[i].fd)->buff))
					{
						if (my_serv.parsing(my_serv.find_user(fds[i].fd)->get_command(), fds[i].fd) == QUIT)
						{
							my_serv.quit(fds[i].fd);
							close_conn = true;
							break;
						}
					}
				}
			/*******************************************************/
			/* If the close_conn flag was turned on, we need       */
			/* to clean up this active connection. This            */
			/* clean up process includes removing the              */
			/* descriptor.                                         */
			/*******************************************************/
			if (close_conn)
			{
				close(fds[i].fd);
				fds[i].fd = -1;
				compress_array = true;
			}


			}  /* End of existing connection is readable             */
		} /* End of loop through pollable descriptors              */

		/***********************************************************/
		/* If the compress_array flag was turned on, we need       */
		/* to squeeze together the array and decrement the number  */
		/* of file descriptors. We do not need to move back the    */
		/* events and revents fields because the events will always*/
		/* be POLLIN in this case, and revents is output.          */
		/***********************************************************/
		
		if (compress_array)
		{
			compress_array = false;
			for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); ++it)
			{
				if (it->fd == -1)
					fds.erase(it--);
			}
		}
		
	}; /* End of serving running.    */

	/*************************************************************/
	/* Clean up all of the sockets that are open                 */
	/*************************************************************/
	for (std::vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
	{
		if ((*it).fd >= 0)
			close((*it).fd);
	}
	fds.clear();
	return 0;
}
