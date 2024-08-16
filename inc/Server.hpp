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
	std::map<int, Connection>& getConnections();
	const std::map<int, Connection>& getConnections() const;
	const std::vector<ServerConfig>& getServerConfigs() const;

	bool registerVirtualServer(int serverFd, const Socket& serverSock);
	bool registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock);

	// SocketPolicy
	struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	int createListeningSocket(const struct addrinfo& addrinfo, int backlog) const;
	Socket retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const;
	int acceptConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;
	ssize_t readFromSocket(int sockfd, char* buffer, size_t size, int flags) const;
	ssize_t writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const;

	// EpollWrapper
	bool addEvent(int newfd, uint32_t eventMask) const;
	bool modifyEvent(int modfd, uint32_t eventMask) const;
	void removeEvent(int delfd) const;

	// RequestParser
	void parseHttpRequest(const std::string& requestString, HTTPRequest& request);
	void clearParser();

	// ResponseBuilder
	void buildResponse(const HTTPRequest& request);
	std::string getResponse();

private:
	const ConfigFile& m_configFile; /**< Global config file */
	EpollWrapper& m_epollWrapper; /**< Wrapper for epoll instance */
	const SocketPolicy& m_socketPolicy; /**< Policy class for socket related functions */
	int m_backlog; /**< Backlog for listening sockets */
	time_t m_clientTimeout; /**< Timeout for a Connection in seconds */
	std::map<int, Socket> m_virtualServers; /**< Listening sockets of virtual servers */
	std::map<int, Connection> m_connections; /**< Current active Connections */
	RequestParser m_requestParser; /**< Handles parsing of request */
	FileSystemPolicy m_fileSystemPolicy; /**< Handles functions for file system manipulation */
	ResponseBuilder m_responseBuilder; /**< Handles building of response */


	Server(const Server& ref);
	Server& operator=(const Server& ref);
};

bool initVirtualServers(Server& server, int backlog, const std::vector<ServerConfig>& serverConfigs);
bool checkDuplicateServer(const Server& server, const std::string& host, const std::string& port);
bool addVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port);

void handleEvent(Server& server, struct epoll_event);
void acceptConnections(Server& server, int serverFd, const Socket& serverSock, uint32_t eventMask);

void handleConnection(Server& server, int clientFd, Connection& connection);
void connectionReceiveRequest(Server& server, int clientFd, Connection& connection);
bool checkForCompleteRequest(const std::string& connectionBuffer);
void connectionReceiveBody(Server& server, int clientFd, Connection& connection);
void connectionBuildResponse(Server& server, int clientFd, Connection& connection);
void connectionSendResponse(Server& server, int clientFd, Connection& connection);
void connectionHandleTimeout(Server& server, int clientFd, Connection& connection);

void handleTimeout(std::map<int, Connection>& connections, time_t clientTimeout, const EpollWrapper& epollWrapper);
