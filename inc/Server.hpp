#pragma once

/* ====== LIBRARIES ====== */

#include "CGIHandler.hpp"
#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "EpollWrapper.hpp"
#include "FileSystemOps.hpp"
#include "Log.hpp"
#include "ProcessOps.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "Socket.hpp"
#include "SocketOps.hpp"
#include "StatusCode.hpp"
#include "TargetResourceHandler.hpp"
#include "constants.hpp"
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
#include <vector>

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
	static const std::size_t s_cgiBodyBufferSize = 32000; /**< Default output buffer size for CGI body in Bytes */

	explicit Server(const ConfigFile& configFile, EpollWrapper& epollWrapper, const FileSystemOps& fileSystemOps,
		const SocketOps& socketOps, const ProcessOps& processOps);
	~Server();

	// Getters

	const std::map<int, Socket>& getVirtualServers() const;
	const std::map<int, Connection>& getConnections() const;
	const std::vector<ConfigServer>& getServerConfigs() const;
	time_t getClientTimeout() const;
	const ProcessOps& getProcessOps() const;

	std::map<int, Socket>& getVirtualServers();
	std::map<int, Connection>& getConnections();
	std::map<int, Connection*>& getCGIConnections();
	std::vector<char>& getClientHeaderBuffer();
	std::vector<char>& getClientBodyBuffer();
	std::vector<char>& getCGIBodyBuffer();

	// Setters
	bool registerVirtualServer(int serverFd, const Socket& serverSock);
	bool registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock);
	bool registerCGIFileDescriptor(int pipeFd, uint32_t eventMask, Connection& connection);
	void removeVirtualServer(int delfd);
	void removeCGIFileDescriptor(int& delfd);
	void setClientTimeout(time_t clientTimeout);

	// Dispatch to EpollWrapper
	int waitForEvents();
	std::vector<struct epoll_event>::const_iterator eventsBegin() const;
	bool addEvent(int newfd, uint32_t eventMask) const;
	bool modifyEvent(int modfd, uint32_t eventMask) const;
	void removeEvent(int delfd) const;
	int getEpollFd() const;

	// Dispatch to SocketOps
	struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	int createListeningSocket(const struct addrinfo* addrinfo, int backlog) const;
	Socket retrieveSocketInfo(struct sockaddr* sockaddr) const;
	Socket retrieveBoundSocketInfo(int sockfd) const;
	int acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;
	ssize_t readFromSocket(int sockfd, char* buffer, size_t size, int flags) const;
	ssize_t writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const;

	// Dispatch to ProcessOps
	ssize_t readProcess(int fileDescriptor, char* buffer, size_t size) const;
	ssize_t writeProcess(int fileDescriptor, const char* buffer, size_t size) const;
	pid_t waitForProcess(pid_t pid, int* wstatus, int options) const;

	// Dispatch to RequestParser
	void parseHeader(const std::string& requestString, HTTPRequest& request);
	void parseBody(const std::string& bodyString, HTTPRequest& request);
	void resetRequestStream();

	// Dispatch to ResponseBuilder
	void buildResponse(Connection& connection);
	std::string getResponse();

	// Dispatch to TargetResourceHandler
	void findTargetResource(Connection& connection);

private:
	const ConfigFile& m_configFile; /**< Global config file */
	EpollWrapper& m_epollWrapper; /**< Wrapper for epoll instance */
	const FileSystemOps& m_fileSystemOps; /**< Handles functions for file system manipulation */
	const SocketOps& m_socketOps; /**< Wrapper for socket-related functions */
	const ProcessOps& m_processOps; /**< Wrapper for process-related functions */

	int m_backlog; /**< Backlog for listening sockets */
	time_t m_clientTimeout; /**< Timeout for a Connection in seconds */
	std::map<int, Socket> m_virtualServers; /**< Listening sockets of virtual servers */
	std::map<int, Connection> m_connections; /**< Current active Connections */
	std::map<int, Connection*> m_cgiConnections; /**< Connections that are currently handling CGI */
	std::vector<char> m_clientHeaderBuffer; /**< Buffer for reading request header */
	std::vector<char> m_clientBodyBuffer; /**< Buffer for reading request body */
	std::vector<char> m_cgiBodyBuffer; /**< Buffer for reading CGI response body */

	RequestParser m_requestParser; /**< Handles parsing of request */
	ResponseBuilder m_responseBuilder; /**< Handles building of response */
	TargetResourceHandler m_targetResourceHandler; /**< Handles target resource of request */

	Server(const Server& ref);
	Server& operator=(const Server& ref);
};

bool initVirtualServers(Server& server, int backlog, const std::vector<ConfigServer>& serverConfigs);
bool isDuplicateServer(const Server& server, const std::string& host, const std::string& port);
bool createVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port);

void runServer(Server& server);
void handleEvent(Server& server, struct epoll_event);

void acceptConnections(Server& server, int serverFd, const Socket& serverSock, uint32_t eventMask);

void handleConnection(Server& server, int activeFd, Connection& connection);

void connectionReceiveHeader(Server& server, int activeFd, Connection& connection);
bool isCompleteRequestHeader(const std::string& connectionBuffer);
void handleCompleteRequestHeader(Server& server, int clientFd, Connection& connection);
bool isCGIRequested(Connection& connection);
void connectionReceiveBody(Server& server, int activeFd, Connection& connection);
void handleBody(Server& server, int activeFd, Connection& connection);
bool isCompleteBody(Connection& connection);
void connectionSendToCGI(Server& server, int activeFd, Connection& connection);
void connectionReceiveFromCGI(Server& server, int activeFd, Connection& connection);
void connectionBuildResponse(Server& server, int activeFd, Connection& connection);
void connectionSendResponse(Server& server, int activeFd, Connection& connection);
void connectionHandleTimeout(Server& server, int activeFd, Connection& connection);

void checkForTimeout(Server& server);

void cleanupClosedConnections(Server& server);
void cleanupIdleConnections(Server& server);

void shutdownServer(Server& server);
