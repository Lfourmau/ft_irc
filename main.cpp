#include "channel.hpp"
#include "server.hpp"

#define SERVER_PORT  6667
using namespace std;

int main ()
{
	int    len, rc, on = 1;
	int    listen_sd = -1, new_sd = -1;
	bool   end_server = false, compress_array = false;
	int    close_conn;
	struct sockaddr_in6   addr;
	vector<struct pollfd> fds;
	int    timeout;

	server my_serv("pass");

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
	rc = ioctl(listen_sd, FIONBIO, (char *)&on);
	if (rc < 0)
	{
		perror("ioctl() failed");
		close(listen_sd);
		exit(-1);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
	addr.sin6_port = htons(SERVER_PORT);
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
		printf("Waiting on poll()...\n");
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
			printf("  poll() timed out.  End program.\n");
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
			if(fds[i].revents != POLLIN)
			{
				printf("  Error! revents = %d\n", fds[i].revents);
				end_server = true;
				break;
			}
			if (fds[i].fd == listen_sd)
			{
				/*******************************************************/
				/* Listening descriptor is readable.                   */
				/*******************************************************/
				printf("  Listening socket is readable\n");

				/*******************************************************/
				/* Accept all incoming connections that are            */
				/* queued up on the listening socket before we         */
				/* loop back and call poll again.                      */
				/*******************************************************/
				sockaddr_in client_addr;
				client_addr.sin_family = AF_INET;
				socklen_t addr_len = sizeof(client_addr);
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
					printf("  New incoming connection - %d\n", new_sd);
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
				printf("  Descriptor %d is readable\n", fds[i].fd);
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
					memset(my_serv.find_user(fds[i].fd).buff, 0, 80);
					rc = recv(fds[i].fd, my_serv.find_user(fds[i].fd).buff, sizeof(my_serv.find_user(fds[i].fd).buff), 0);
					std::cout << "{" << my_serv.find_user(fds[i].fd).buff << "}" << std::endl;
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
						printf("  Connection closed\n");
						close_conn = true;
						break;
					}

					/*****************************************************/
					/* Data was received                                 */
					/* Parsing changes                                   */
					/*****************************************************/
					len = rc;
					printf("  %d bytes received\n", len);
					//parse instead of echo data to the client
					if (my_serv.find_user(fds[i].fd).set_command(my_serv.find_user(fds[i].fd).buff))
						my_serv.parsing(my_serv.find_user(fds[i].fd).get_command(), fds[i].fd);
		
					/*****************************************************/
					/* Echo the data back to the client                  */
					/*****************************************************/
					//rc = send(fds[i].fd, buffer, len, 0);
					//if (rc < 0)
					//{
					//	perror("  send() failed");
					//	close_conn = true;
					//	break;
					//}
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
			for (vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
			{
			if ((*it).fd == -1)
				fds.erase(it);
			}
		}
	}; /* End of serving running.    */

	/*************************************************************/
	/* Clean up all of the sockets that are open                 */
	/*************************************************************/
	for (vector<struct pollfd>::iterator it = fds.begin(); it != fds.end(); it++)
	{
		if ((*it).fd >= 0)
			close((*it).fd);
	}
	fds.clear();
	return 0;
}