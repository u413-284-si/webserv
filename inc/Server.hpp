#ifndef SERVER_HPP
# define SERVER_HPP

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <algorithm>
#include <errno.h>

#define MAX_EVENTS 10
#define PORT 8080

class Server{
    private:
                int server_sock, client_sock, epfd, nfds;
	            struct sockaddr_in client_addr;
	            struct epoll_event ev, events[MAX_EVENTS];

    public:
                Server();
                ~Server();

                void    run();
                void    acceptConnection();
                void    handleConnections(int index);
};

#endif
