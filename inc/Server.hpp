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
#include "error.hpp"
#include "signalHandler.hpp"

#include <algorithm>
#include <bits/types/time_t.h>
#include <cassert>
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
public:
	static const int s_backlog = 10; /**< Default backlog for listening sockets */
	static const time_t s_clientTimeout = 60; /**< Default timeout for a Connection in seconds */
	static const std::size_t s_bufferSize = 1024; /**< Default buffer size for reading from sockets in Bytes */
	static const std::size_t s_clientHeaderBufferSize = 1000; /**< Default buffer size for request header in Bytes */
	static const std::size_t s_clientBodyBufferSize = 16000; /**< Default buffer size for request body in Bytes */
	static const std::size_t s_clientMaxBodySize = 1000000; /**< Default max size for request body in Bytes */

	explicit Server(const ConfigFile& configFile, EpollWrapper& epollWrapper, const SocketPolicy& socketPolicy);
	~Server();

	// Getters
	std::map<int, Socket>& getVirtualServers();
	const std::map<int, Socket>& getVirtualServers() const;
	std::map<int, Connection>& getConnections();
	const std::map<int, Connection>& getConnections() const;
	const std::vector<ConfigServer>& getServerConfigs() const;
	time_t getClientTimeout() const;

	// Setters
	bool registerVirtualServer(int serverFd, const Socket& serverSock);
	bool registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock);
	void setClientTimeout(time_t clientTimeout);

	// Dispatch to EpollWrapper
	int waitForEvents();
	std::vector<struct epoll_event>::const_iterator eventsBegin() const;
	bool addEvent(int newfd, uint32_t eventMask) const;
	bool modifyEvent(int modfd, uint32_t eventMask) const;
	void removeEvent(int delfd) const;

	// Dispatch to SocketPolicy
	struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	int createListeningSocket(const struct addrinfo* addrinfo, int backlog) const;
	Socket retrieveSocketInfo(struct sockaddr* sockaddr, socklen_t socklen) const;
	int acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;
	ssize_t readFromSocket(int sockfd, char* buffer, size_t size, int flags) const;
	ssize_t writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const;

	// Dispatch to RequestParser
	void parseHeader(const std::string& requestString, HTTPRequest& request);
	void parseBody(const std::string& bodyString, HTTPRequest& request);
	void resetRequestStream();

	// Dispatch to ResponseBuilder
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

bool initVirtualServers(Server& server, int backlog, const std::vector<ConfigServer>& serverConfigs);
bool isDuplicateServer(const Server& server, const std::string& host, const std::string& port);
bool createVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port);

void runServer(Server& server);
void handleEvent(Server& server, struct epoll_event);

void acceptConnections(Server& server, int serverFd, const Socket& serverSock, uint32_t eventMask);

void handleConnection(Server& server, int clientFd, Connection& connection);
void connectionReceiveHeader(Server& server, int clientFd, Connection& connection);
bool isCompleteRequestHeader(const std::string& connectionBuffer);
void connectionReceiveBody(Server& server, int clientFd, Connection& connection);
bool isCompleteBody(Connection& connection);
void connectionBuildResponse(Server& server, int clientFd, Connection& connection);
void connectionSendResponse(Server& server, int clientFd, Connection& connection);
void connectionHandleTimeout(Server& server, int clientFd, Connection& connection);

void checkForTimeout(Server& server);

void cleanupClosedConnections(Server& server);
void cleanupIdleConnections(Server& server);

void shutdownServer(Server& server);
