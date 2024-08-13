#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "EpollWrapper.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Socket.hpp"
#include "SocketPolicy.hpp"
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
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

/* ====== CLASS DECLARATION ====== */

/**
 * @brief Main class of the HTTP server.
 *
 * This class is responsible for the main loop of the server. It handles the epoll instance and the
 * listening sockets of the virtual servers. It also handles the connections and the timeout of the
 * connections.
 * @todo Remove s_backlog and m_backlog and use value from the config file.
 */
class Server {
private:
	static const int s_backlog = 10; /**< Default backlog for listening sockets */
	static const time_t s_clientTimeout = 60; /**< Default timeout for a Connection in seconds */
	static const std::size_t s_bufferSize = 1024; /**< Default buffer size for reading from sockets in Bytes */

public:
	explicit Server(const ConfigFile& configFile, EpollWrapper& epollWrapper, const SocketPolicy& socketPolicy);
	~Server();

	void run();

	const std::map<int, Socket>& getVirtualServers() const;
	const std::map<int, Connection>& getConnections() const;

	bool registerVirtualServer(int serverFd, const Socket& serverSock);
	bool registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock);

	struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	int createListeningSocket(const struct addrinfo& addrinfo, int backlog) const;
	Socket retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const;
	int acceptConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;

private:
	const ConfigFile& m_configFile; /**< Global config file */
	EpollWrapper& m_epollWrapper; /**< Wrapper for epoll instance */
	const SocketPolicy& m_socketPolicy; /**< Policy class for socket related functions */
	int m_backlog; /**< Backlog for listening sockets */
	time_t m_clientTimeout; /**< Timeout for a Connection in seconds */
	std::map<int, Socket> m_virtualServers; /**< Listening sockets of virtual servers */
	std::map<int, Connection> m_connections; /**< Current active Connections */
	std::map<int, std::string> m_connectionBuffers; /**< Buffers for active Connections */
	RequestParser m_requestParser; /**< Handles parsing of request */
	FileSystemPolicy m_fileSystemPolicy; /**< Handles functions for file system manipulation */
	ResponseBuilder m_responseBuilder; /**< Handles building of response */

	void handleEvent(struct epoll_event);

	void handleConnections(int clientFd, const Connection& connection);

	Server(const Server& ref);
	Server& operator=(const Server& ref);
};

void acceptConnections(Server& server, int serverFd, const Socket& serverSock, uint32_t eventMask);

bool initVirtualServers(Server& server, int backlog, const std::vector<ServerConfig>& serverConfigs);
bool checkDuplicateServer(const Server& server, const std::string& host, const std::string& port);
bool addVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port);

bool checkForCompleteRequest(const std::string& connectionBuffer);

void handleTimeout(std::map<int, Connection>& connections, time_t clientTimeout, const EpollWrapper& epollWrapper);
