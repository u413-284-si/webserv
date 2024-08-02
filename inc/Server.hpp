#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Socket.hpp"
#include "StatusCode.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <utility>

/* ====== CLASS DECLARATION ====== */

class Server {
private:
	static const int s_epollTimeout = 1000;
	static const int s_epollMaxEvents = 10;
	static const int s_backlog = 10;
	static const time_t s_clientTimeout = 60;
	static const std::size_t s_bufferSize = 8192;

public:
	explicit Server(
		const ConfigFile& configFile, int epollTimeout = s_epollTimeout, size_t maxEvents = s_epollMaxEvents);
	~Server();

	void run();

private:
	const ConfigFile& m_configFile; /**< Global config file */
	int m_epfd; /**< FD of epoll instance */
	int m_epollTimeout; /**< Timeout for epoll instance */
	std::vector<struct epoll_event> m_epollEvents; /**< Holds epoll events */
	int m_backlog; /**< Backlog for listening sockets */
	time_t m_clientTimeout; /**< Timeout for a Connection */
	std::size_t m_bufferSize; /**< Size of buffer for reading requests */
	std::map<int, Socket> m_virtualServers; /**< Listening sockets of virtual servers */
	std::map<int, Connection> m_connections; /**< Current active Connections */
	std::map<int, std::string> m_requestStrings; /**< Buffers for active Connections */
	RequestParser m_requestParser; /**< Handles parsing of request */
	FileSystemPolicy m_fileSystemPolicy; /**< Handles functions for file system manipulation */
	ResponseBuilder m_responseBuilder; /**< Handles building of response */

	bool init();
	bool addVirtualServer(const std::string& host, int backlog, const std::string& port);

	void acceptConnections(const Socket& serverSock, uint32_t eventMask);
	void handleConnections(const Connection& connection);
	void handleTimeout();

	bool checkDuplicateServer(const std::string& host, const std::string& port);
	bool checkForCompleteRequest(int clientSock);
	bool registerVirtualServer(const Socket& serverSock);
	bool registerConnection(const Socket& serverSock, const Socket& clientSock);



	Server(const Server& ref);
	Server& operator=(const Server& ref);
};

int createListeningSocket(struct addrinfo* addrinfo, int backlog);
Socket retrieveSocketInfo(int sockFd, const struct sockaddr* sockaddr, socklen_t socklen);

bool addEvent(int epfd, int newfd, epoll_event* event);
void removeEvent(int epfd, int delfd);
bool modifyEvent(int epfd, int modfd, epoll_event* event);
