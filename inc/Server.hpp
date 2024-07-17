#pragma once

/* ====== LIBRARIES ====== */

#include "RequestParser.hpp"
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

/* ====== DEFINITIONS ====== */

#define MAX_EVENTS 10
#define PORT 8080
#define BUFFER_SIZE 1024

/* ====== CLASS DECLARATION ====== */

class Server {
private:
	int m_serverSock;
	int m_epfd;
	std::map<int, std::string> m_requestStrings;

	void acceptConnection();
	void handleConnections(int clientSock, RequestParser& parser);
	bool checkForCompleteRequest(int clientSock);

public:
	Server();
	~Server();

	void run();
};
