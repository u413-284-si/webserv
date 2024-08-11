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
	void handleTimeout();

	Server(const Server& ref);
	Server& operator=(const Server& ref);
};

void acceptConnections(const SocketPolicy& socketPolicy, const EpollWrapper& epollWrapper, std::map<int, Connection>& connections,
	std::map<int, std::string>& connectionBuffers, int serverFd, const Socket& serverSock, uint32_t eventMask);
bool registerConnection(const EpollWrapper& epollWrapper, std::map<int, Connection>& connections,
	std::map<int, std::string>& connectionBuffers, const Socket& serverSock, int clientFd, const Socket& clientSock);

bool initVirtualServers(const std::vector<ServerConfig>& serverConfigs, const SocketPolicy& socketPolicy,
	const EpollWrapper& epollWrapper, std::map<int, Socket>& virtualServers, int backlog);
bool checkDuplicateServer(
	const std::map<int, Socket>& virtualServers, const std::string& host, const std::string& port);
bool addVirtualServer(const SocketPolicy& socketPolicy, const EpollWrapper& epollWrapper,
	std::map<int, Socket>& virtualServers, const std::string& host, int backlog, const std::string& port);
bool registerVirtualServer(
	const EpollWrapper& epollWrapper, std::map<int, Socket>& virtualServers, int serverFd, const Socket& serverSock);

bool checkForCompleteRequest(const std::string& connectionBuffer);

