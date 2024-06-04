#ifndef SERVER_HPP
# define SERVER_HPP

/* ====== LIBRARIES ====== */

#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <algorithm>
#include <errno.h>

/* ====== DEFINITIONS ====== */

#define MAX_EVENTS 10
#define PORT 8080
#define BUFFER_SIZE 1024

/* ====== CLASS DECLARATION ====== */

class Server{
    private:
                int					_serverSock;
				int					_epfd;
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
