#include "Server.hpp"
#include "ConfigFile.hpp"
#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/epoll.h>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.

 * The Server constructor initializes a Server object with the provided configuration file, epoll
 * timeout, and maximum events, along with other member variables.
 * It creates an epoll instance using the epoll_create function and initializes the epoll events vector
 * with the maximum number of events that can be processed by the epoll instance.
 * It also initializes the virtual servers using the configuration file with Server::initVirtualServers().
 *
 * @param configFile The `configFile` parameter is an object of type `ConfigFile` that is passed to the
 * `Server` constructor. It is used to configure the server with settings such especially the number and
 * configuration of virtual servers.
 * @param epollTimeout The `epollTimeout` parameter in the `Server` constructor represents the timeout
 * value in milliseconds for the `epoll_wait` function. This function is used for waiting for events on
 * an epoll instance. The `epoll_wait` function will block for this duration if no events are available
 * before returning
 * @param maxEvents The `maxEvents` parameter in the `Server` constructor represents the maximum number
 * of events that can be processed by the epoll instance. This value determines the size of the event
 * vector used by epoll to store events that are ready for processing. It is used to allocate memory for
 * the array that holds the events returned by `epoll_wait`.

 * @throws std::runtime_error if epoll_create() or Server::initVirtualServers() fail.
 * @todo Several variables are init to static ones, could be passed as parameters or set in config file.
 */
Server::Server(const ConfigFile& configFile, EpollWrapper& epollWrapper, const SocketPolicy& socketPolicy)
	: m_configFile(configFile)
	, m_epollWrapper(epollWrapper)
	, m_socketPolicy(socketPolicy)
	, m_backlog(s_backlog)
	, m_clientTimeout(s_clientTimeout)
	, m_responseBuilder(m_configFile, m_fileSystemPolicy)
{
}

/**
 * @brief Destructor for the Server class.
 *
 * This destructor cleans up resources associated with the server instance by
 * closing all open virtual server sockets, all open connections and the epoll instance.
 *
 * 1. Closes all virtual server sockets to release the bound port and stop accepting
 * new connections.
 * 2. Closes all open connections to release associated resources.
 * 3. Closes the epoll instance to release associated resources and stop
 * monitoring events.
 * @todo Is it possible to send a last message to all connected clients to notify server shutdown?
 */
Server::~Server()
{
	for (std::map<int, Socket>::iterator iter = m_virtualServers.begin(); iter != m_virtualServers.end(); ++iter)
		close(iter->first);

	for (std::map<int, Connection>::iterator iter = m_connections.begin(); iter != m_connections.end(); ++iter) {
		m_epollWrapper.removeEvent(iter->first);
		close(iter->first);
	}
}

/* ====== MEMBER FUNCTIONS ====== */

/**
 * @brief Run the server event loop.
 *
 * This method enters a continuous loop to handle incoming events using epoll.
 * It waits for events using epoll_wait, processes the events, and dispatches
 * them to appropriate handler functions.
 *
 * @throws std::runtime_error if epoll_wait encounters an error.
 *
 */
void Server::run()
{
	LOG_INFO << "Server started";

	while (true) {
		const int nfds = m_epollWrapper.waitForEvents();

		for (std::vector<struct epoll_event>::const_iterator iter = m_epollWrapper.eventsBegin();
			 iter != m_epollWrapper.eventsBegin() + nfds; ++iter) {
			handleEvent(*this, *iter);
		}
		checkForTimeout(*this);
		cleanupClosedConnections(*this);
	}
}

/**
 * @brief Initialize virtual servers.
 *
 * Initializes virtual servers by iterating through the serverConfigs
 * vector in the configuration file and open a socket for each virtual server.
 * It checks for duplicate servers using checkDuplicateServer() and
 * skips opening the socket server if it already exists.
 *
 * @return true if at least one virtual server was added, false otherwise.
 */
bool initVirtualServers(Server& server, int backlog, const std::vector<ServerConfig>& serverConfigs)
{
	LOG_INFO << "Initializing virtual servers";

	for (std::vector<ServerConfig>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {

		LOG_DEBUG << "Adding virtual server: " << iter->serverName << " on " << iter->host << ":" << iter->port;

		if (checkDuplicateServer(server, iter->host, webutils::toString(iter->port)))
			continue;

		if (!addVirtualServer(server, iter->host, backlog, webutils::toString(iter->port))) {
			LOG_DEBUG << "Failed to add virtual server: " << iter->serverName;
			continue;
		}
	}

	if (server.getVirtualServers().empty())
		return false;

	LOG_INFO << "Finished setting up virtual servers";
	return true;
}

/**
 * @brief Add a virtual server.
 *
 * Creates a listening socket for a virtual server using the provided host, port, and backlog and adds it to the
 * virtual servers map.
 * Uses resolveListeningAddresses() to get a list of address information for the provided host and port.
 * It then tries to create a listening socket for each value in the list using createListeningSocket(). If succesful
 * calls retrieveSocketInfo() and registers the virtual server using Server::registerVirtualServer(). If any step fails,
 * it continues to the next value in the list.
 * After the loop, address list is freed.
 * If no valid socket was created, it logs an error and returns false.
 *
 * @param host host address for the virtual server.
 * @param backlog maximum length to which the queue of pending connections for the virtual server may grow.
 * @param port port number for the virtual server.
 *
 * @return true if the virtual server was successfully added, false otherwise.
 */
bool addVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port)
{
	struct addrinfo* list = server.resolveListeningAddresses(host, port);
	if (list == NULL)
		return false;

	size_t successfulSock = 0;
	size_t countTryCreateSocket = 1;
	for (struct addrinfo* curr = list; curr != NULL; curr = curr->ai_next) {
		LOG_DEBUG << countTryCreateSocket << ". try to create listening socket";

		const int newFd = server.createListeningSocket(*curr, backlog);
		if (newFd == -1)
			continue;

		const Socket serverSock = server.retrieveSocketInfo(*curr->ai_addr, curr->ai_addrlen);
		if (serverSock.host.empty() && serverSock.port.empty()) {
			close(newFd);
			continue;
		}

		if (!server.registerVirtualServer(newFd, serverSock))
			continue;

		++successfulSock;
	}
	freeaddrinfo(list);

	if (successfulSock == 0) {
		LOG_ERROR << "Cannot bind to a valid socket.";
		return false;
	}

	return true;
}

/**
 * @brief Accepts new connections or handles existing ones

 * If the event mask contains EPOLLERR or EPOLLHUP, the event mask is set to EPOLLIN.
 * This is done to handle errors and hangups that may occur during the processing of the connection.
 * The Server could then try to recv(), which will return 0 in case of EPOLLHUP or -1 in case of EPOLLERR,
 * resulting in a connection close.
 * @sa https://stackoverflow.com/a/29206631
 *
 * Checks if the event file descriptor is in the virtual servers map. If it is, it calls acceptConnections().
 * If it is not, it calls handleConnections() with the connection from the connections map.
 * A connection should exist in the connections map, therefore it is not checked if the file descriptor
 * is in the connections map.
 *
 * @param event The epoll event to handle.
 */
void handleEvent(Server& server, struct epoll_event event)
{
	uint32_t eventMask = event.events;

	std::map<int, Socket>::const_iterator iter = server.getVirtualServers().find(event.data.fd);
	if (iter != server.getVirtualServers().end()) {
		if ((eventMask & EPOLLERR) != 0) {
			LOG_ERROR << "Error condition happened on the associated file descriptor of " << iter->second;
			// what to do here?
			return;
		}
		acceptConnections(server, iter->first, iter->second, eventMask);
	} else {
		if ((eventMask & EPOLLERR) != 0) {
			LOG_DEBUG << "epoll_wait: EPOLLERR";
			eventMask = EPOLLIN;
		} else if ((eventMask & EPOLLHUP) != 0) {
			LOG_DEBUG << "epoll_wait: EPOLLHUP";
			eventMask = EPOLLIN;
		}
		handleConnection(server, event.data.fd, server.getConnections().at(event.data.fd));
	}
}

/**
 * @brief Accept new connections
 *
 * If the event mask does not contain `EPOLLIN`, it logs an error and returns.
 *
 * In a loop, calls accept() to accept new connections on the server socket.
 * When accept() is successful, information of the client socket is retrieved with retrieveSocketInfo().
 * This new client socket is then registered with registerConnection().
 *
 * The while-loop allows for multiple connections to be accepted in one call
 * of this function. It is broken when accept sets errno to EAGAIN or
 * EWOULDBLOCK (equivalent error codes indicating that a non-blocking operation
 * would normally block), meaning that no more connections are pending.
 * If another errno is returned logs an error and continues to the next
 * iteration of the loop. This is because the server sockets are in EPOLLET mode.
 *
 * If the event mask contains EPOLLERR, it logs an error and closes the server socket.
 * The server socket is then removed from the virtual servers map.
 *
 * @param serverFd File descriptor of the server socket.
 * @param serverSock Server socket which reported an event.
 * @param eventMask Event mask of the reported event.
 */
void acceptConnections(Server& server, int serverFd, const Socket& serverSock, uint32_t eventMask)
{
	LOG_DEBUG << "Accept connections on: " << serverSock;

	if ((eventMask & EPOLLIN) == 0) {
		LOG_ERROR << "Received unknown event:" << eventMask;
		return;
	}

	while (true) {
		struct sockaddr_storage clientAddr = {};
		socklen_t clientLen = sizeof(clientAddr);

		// NOLINTNEXTLINE: we need to use reinterpret_cast to convert sockaddr_storage to sockaddr
		struct sockaddr* addrCast = reinterpret_cast<struct sockaddr*>(&clientAddr);

		const int clientFd = server.acceptConnection(serverFd, addrCast, &clientLen);
		if (clientFd == -2)
			return; // No more pending connections
		if (clientFd == -1)
			continue; // Error accepting connection

		const Socket clientSock = server.retrieveSocketInfo(*addrCast, clientLen);
		if (clientSock.host.empty() && clientSock.port.empty()) {
			close(clientFd);
			continue;
		}

		if (!server.registerConnection(serverSock, clientFd, clientSock))
			continue;
	}
}

/**
 * @brief Handle data from a client connection.
 *
 * This method is responsible for reading data from a client socket
 * and processing it.
 *
 * @param clientSock The file descriptor of the event in the epoll events array.
 *
 * @details The method performs the following steps:
 *
 * 1. Retrieves the file descriptor of the client socket from the epoll events array.
 * 2. Enters a loop to continuously read data from the client socket.
 * 3. Reads data from the client socket using the read function into a buffer.
 *    - If bytesRead is less than 0, it indicates that there is no more data to be read
 *      or an error occurred with the read operation, and the loop breaks.
 *    - If bytesRead is 0, it indicates that the connection has been closed by the client,
 *      so the method closes the client socket and exits the loop.
 *    - Otherwise, the method echoes the received data back to the client by writing it
 *      to the client socket using the write function.
 *
 */
void handleConnection(Server& server, const int clientFd, Connection& connection)
{
	LOG_DEBUG << "Handling connection: " << connection.m_clientSocket << " for server: " << connection.m_serverSocket;

	switch (connection.m_status) {
	case (Connection::ReceiveRequest):
		connectionReceiveRequest(server, clientFd, connection);
		break;
	case (Connection::ReceiveBody):
		connectionReceiveBody(server, clientFd, connection);
		break;
	case (Connection::BuildResponse):
		connectionBuildResponse(server, clientFd, connection);
		break;
	case (Connection::SendResponse):
		connectionSendResponse(server, clientFd, connection);
		break;
	case (Connection::Timeout):
		connectionHandleTimeout(server, clientFd, connection);
		break;
	case (Connection::Closed):
		break;
	}
}

void connectionReceiveRequest(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "ReceiveRequest for: " << connection.m_clientSocket;

	const size_t bufferSize = 1000;
	char buffer[bufferSize];
	const size_t bytesToRead = bufferSize - connection.m_bytesReceived;

	const ssize_t bytesRead = server.readFromSocket(clientFd, buffer, bytesToRead, 0);
	if (bytesRead < 0) {
		// Internal server error
		close(clientFd);
		connection.m_status = Connection::Closed;
	} else if (bytesRead == 0) {
		// Connection closed by client
		close(clientFd);
		connection.m_status = Connection::Closed;
	} else {
		connection.m_bytesReceived += bytesRead;
		connection.m_buffer += buffer;
		if (checkForCompleteRequest(connection.m_buffer)) {
			LOG_DEBUG << "Received complete request: " << '\n' << connection.m_buffer;
			try {
				server.parseHttpRequest(connection.m_buffer, connection.m_request);
				server.clearParser();
			} catch (std::exception& e) {
				LOG_ERROR << "Error: " << e.what();
			}
			connection.m_buffer.erase(0, connection.m_buffer.find("\r\n\r\n") + 4);
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(clientFd, EPOLLOUT);
		} else {
			LOG_DEBUG << "Received partial request: " << '\n' << connection.m_buffer;
			if (connection.m_bytesReceived == bufferSize) {
				LOG_ERROR << "Buffer full, closing connection";
				close(clientFd);
				connection.m_status = Connection::Closed;
			}
		}
	}
	connection.m_timeSinceLastEvent = std::time(0);
}

void connectionReceiveBody(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "ReceiveBody for: " << connection.m_clientSocket;

	(void)server;
	(void)clientFd;
}

void connectionBuildResponse(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "BuildResponse for: " << connection.m_clientSocket;

	server.buildResponse(connection.m_request);
	connection.m_buffer = server.getResponse();
	connection.m_status = Connection::SendResponse;
	connectionSendResponse(server, clientFd, connection);
}

void connectionSendResponse(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "SendResponse for: " << connection.m_clientSocket;

	const ssize_t bytesToSend = static_cast<ssize_t>(connection.m_buffer.size());
	const ssize_t sentBytes = server.writeToSocket(clientFd, connection.m_buffer.c_str(), bytesToSend, 0);
	if (sentBytes == -1) {
		LOG_ERROR << "Internal server error";
		close(clientFd);
		connection.m_status = Connection::Closed;
		return;
	}

	if (sentBytes < bytesToSend) {
		LOG_DEBUG << "Sent " << sentBytes << " bytes";
		connection.m_buffer.erase(0, sentBytes);
		connection.m_timeSinceLastEvent = std::time(0);
		return;
	}

	if (connection.m_request.shallCloseConnection) {
		LOG_DEBUG << "Closing connection";
		close(clientFd);
		connection.m_status = Connection::Closed;
	} else {
		LOG_DEBUG << "Connection alive";
		server.modifyEvent(clientFd, EPOLLIN);
		clearConnection(connection);
	}
}

void connectionHandleTimeout(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "Timeout for: " << connection.m_clientSocket;

	connection.m_request.shallCloseConnection = true;
	connection.m_request.httpStatus = StatusRequestTimeout;

	connectionBuildResponse(server, clientFd, connection);
}

/**
 * @brief Iterates through all connections and closes any that have timed out.

 * The for loop through the connections map has no increment statement because the
 * iterator is incremented in the loop body. If .erase() is called on an iterator,
 * it is invalidated.
 * The time since last event is saved in a variable to print it to the log and check
 * if it is greater than m_clientTimeout. If it is, the connection is removed from epoll,
 * and closed by calling Connection::closeConnection(). This makes sure that the connection
 * is properly closed.
 * @todo Send a message to client in case of timeout?
 */
void checkForTimeout(Server& server)
{
	for (std::map<int, Connection>::iterator iter = server.getConnections().begin();
		 iter != server.getConnections().end(); ++iter) {
		if (iter->second.m_status == Connection::Closed)
			continue;
		const time_t timeSinceLastEvent = std::time(0) - iter->second.m_timeSinceLastEvent;
		LOG_DEBUG << iter->second.m_clientSocket << ": Time since last event: " << timeSinceLastEvent;
		if (timeSinceLastEvent > server.getClientTimeout()) {
			LOG_INFO << "Connection timeout: " << iter->second.m_clientSocket;
			server.modifyEvent(iter->first, EPOLLOUT);
			iter->second.m_status = Connection::Timeout;
		}
	}
}

bool checkForCompleteRequest(const std::string& connectionBuffer)
{
	const size_t headerEndPos = connectionBuffer.find("\r\n\r\n");

	return (headerEndPos != std::string::npos);
	/*
	headerEndPos += 4;
	size_t bodySize = m_requestStrings[clientSock].size() - headerEndPos;
	// FIXME: add check against default/config max body size
	size_t contentLengthPos = m_requestStrings[clientSock].find("Content-Length");
	size_t transferEncodingPos = m_requestStrings[clientSock].find("Transfer-Encoding");

	if (contentLengthPos != std::string::npos && transferEncodingPos == std::string::npos) {
		unsigned long contentLength
			= std::strtoul(m_requestStrings[clientSock].c_str() + contentLengthPos + 15, NULL, 10);
		if (bodySize >= contentLength)
			return true;
	} else if (transferEncodingPos != std::string::npos) {
		std::string tmp = m_requestStrings[clientSock].substr(transferEncodingPos);
		if (tmp.find("chunked") != std::string::npos && tmp.find("0\r\n\r\n") != std::string::npos)
			return true;
	}
	*/
}

/**
 * @brief Register a virtual server.
 *
 * Adds a virtual server to the list of virtual servers and registers it with epoll.
 *
 * @param serverSock The socket of the virtual server to register.
 *
 * @return true if the virtual server was successfully registered, false otherwise.
 */
bool Server::registerVirtualServer(int serverFd, const Socket& serverSock)
{
	if (!m_epollWrapper.addEvent(serverFd, EPOLLIN | EPOLLET)) {
		close(serverFd);
		LOG_ERROR << "Failed to add event for " << serverSock;
		return false;
	}

	m_virtualServers[serverFd] = serverSock;

	LOG_INFO << "New virtual server: " << serverSock;
	return true;
}

/**
 * @brief Register a connection.
 *
 * Registers a client fd with epoll.
 * If it fails to add the event, it logs an error and closes the client socket.
 * If the connection is successfully registered, adds the connection to map m_connections.
 * The []-operator creates a new entry in the map if the key does not exist or overwrites an existing one.
 * The same way (re-)initializes the map m_connectionBuffers[newFD] to an empty string.
 *
 * @param serverSock The socket of the server that the connection is associated with.
 * @param clientSock The socket of the client connection to register.
 *
 * @return true if the connection was successfully registered, false otherwise.
 */
bool Server::registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock)
{
	if (!m_epollWrapper.addEvent(clientFd, EPOLLIN)) {
		close(clientFd);
		LOG_ERROR << "Failed to add event for " << clientSock;
		return false;
	}

	m_connections[clientFd] = Connection(serverSock, clientSock);

	LOG_INFO << "New Connection: " << clientSock << " for server: " << serverSock;
	return true;
}

/* ====== HELPER FUNCTIONS ====== */

/**
 * @brief Check for duplicate virtual servers.
 *
 * Checks if a virtual server with the provided host and port already exists in the virtual servers map.
 * If the host is "localhost", it checks for a virtual server with the host address "127.0.0.1" or "::1".
 *
 * @param virtualServers The map of virtual servers.
 * @param host The host address of the virtual server.
 * @param port The port number of the virtual server.
 *
 * @return true if a duplicate virtual server exists, false otherwise.
 */
bool checkDuplicateServer(const Server& server, const std::string& host, const std::string& port)
{
	if (host == "localhost") {
		for (std::map<int, Socket>::const_iterator iter = server.getVirtualServers().begin();
			 iter != server.getVirtualServers().end(); ++iter) {
			if ((iter->second.host == "127.0.0.1" || iter->second.host == "::1") && iter->second.port == port) {
				LOG_DEBUG << "Virtual server already exists: " << iter->second;
				return true;
			}
		}
	} else {
		for (std::map<int, Socket>::const_iterator iter = server.getVirtualServers().begin();
			 iter != server.getVirtualServers().end(); ++iter) {
			if (iter->second.host == host && iter->second.port == port) {
				LOG_DEBUG << "Virtual server already exists: " << iter->second;
				return true;
			}
		}
	}
	return false;
}

struct addrinfo* Server::resolveListeningAddresses(const std::string& host, const std::string& port) const
{
	return m_socketPolicy.resolveListeningAddresses(host, port);
}

int Server::createListeningSocket(const struct addrinfo& addrinfo, int backlog) const
{
	return m_socketPolicy.createListeningSocket(addrinfo, backlog);
}

Socket Server::retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const
{
	return m_socketPolicy.retrieveSocketInfo(sockaddr, socklen);
}

int Server::acceptConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const
{
	return m_socketPolicy.acceptConnection(sockfd, addr, addrlen);
}

ssize_t Server::readFromSocket(int sockfd, char* buffer, size_t size, int flags) const
{
	return m_socketPolicy.readFromSocket(sockfd, buffer, size, flags);
}

ssize_t Server::writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const
{
	return m_socketPolicy.writeToSocket(sockfd, buffer, size, flags);
}

bool Server::addEvent(int newfd, uint32_t eventMask) const { return m_epollWrapper.addEvent(newfd, eventMask); }

bool Server::modifyEvent(int modfd, uint32_t eventMask) const { return m_epollWrapper.modifyEvent(modfd, eventMask); }

void Server::removeEvent(int delfd) const { m_epollWrapper.removeEvent(delfd); }

const std::map<int, Socket>& Server::getVirtualServers() const { return m_virtualServers; }

std::map<int, Connection>& Server::getConnections() { return m_connections; }

const std::map<int, Connection>& Server::getConnections() const { return m_connections; }

const std::vector<ServerConfig>& Server::getServerConfigs() const { return m_configFile.serverConfigs; }

time_t Server::getClientTimeout() const { return m_clientTimeout; }

void Server::parseHttpRequest(const std::string& requestString, HTTPRequest& request)
{
	m_requestParser.parseHttpRequest(requestString, request);
}

void Server::clearParser() { m_requestParser.clearParser(); }

void Server::buildResponse(const HTTPRequest& request) { m_responseBuilder.buildResponse(request); }

std::string Server::getResponse() { return m_responseBuilder.getResponse(); }

void cleanupClosedConnections(Server& server)
{
	for (std::map<int, Connection>::iterator iter = server.getConnections().begin();
		 iter != server.getConnections().end();
		/* no iter*/) {
		if (iter->second.m_status == Connection::Closed)
			server.getConnections().erase(iter++);
		else
			++iter;
	}
}
