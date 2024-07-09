#pragma once

/* ====== LIBRARIES ====== */

#include "RequestParser.hpp"
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
#include <string.h>
#include <map>

/* ====== DEFINITIONS ====== */

#define MAX_EVENTS 10
#define PORT 8080
#define BUFFER_SIZE 1024

/* ====== CLASS DECLARATION ====== */

class Server{
    private:
                int							m_serverSock;
				int							m_epfd;
				std::map<int,std::string>	m_requestStrings;

				void    acceptConnection();
                void    handleConnections(int clientSock);
				bool	checkForCompleteRequest(int clientSock);

    public:
                Server();
                ~Server();

                void    run();
};
