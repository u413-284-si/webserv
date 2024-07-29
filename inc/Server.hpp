#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Log.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "StatusCode.hpp"

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
