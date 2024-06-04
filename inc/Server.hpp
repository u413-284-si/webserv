#ifndef SERVER_HPP
# define SERVER_HPP

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <algorithm>
#include <errno.h>
#include <stdexcept>
#include <iostream>

#define MAX_EVENTS 10
#define PORT 8080

class Server{
    private:
                int					_serverSock;
				int					_clientSock;
				int					_epfd;
				int					_nfds;
	            struct sockaddr_in	_clientAddr;
	            struct epoll_event	_ev;
				struct epoll_event	_events[MAX_EVENTS];

    public:
                Server();
                ~Server();

                void    run();
                void    acceptConnection();
                void    handleConnections(int index);
};

#endif
