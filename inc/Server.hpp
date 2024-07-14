#pragma once

/* ====== LIBRARIES ====== */

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

/* ====== DEFINITIONS ====== */

#define MAX_EVENTS 10
#define PORT 8080
#define BUFFER_SIZE 1024
#define CONNECTION_QUEUE 10

/* ====== CLASS DECLARATION ====== */

class Server {
private:
	Server(const Server& ref);
	Server& operator=(const Server& ref);

	int m_serverSock;
	int m_epfd;
	std::map<int, std::string> m_requestStrings;

public:
	Server();
	~Server();

	void run();
	void acceptConnection();
	void handleConnections(int clientSock);
};
