#include "Server.hpp"
#include "StatusCode.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.

 * The Server constructor initializes a Server object with the provided configuration file, EpollWrapper, and
 * SocketPoolicy, along with other member variables.
 * The passed EpollWrapper is saved as a non-const ref as it needs to be modifiable.
 * The passed SocketPolicy is saved as a const ref.
 *
 * @param configFile The `configFile` parameter is an object of type `ConfigFile` that is passed to the
 * `Server` constructor. It is used to configure the server with settings such especially the number and
 * configuration of virtual servers.
 * @param epollWrapper A ready to use epoll instance. Can be mocked for testing.
 * @param socketPolicy Policy class for functions related to mocking. Can be mocked for testing.

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
 * @brief Runs the main event loop.
 *
 * This functions enters a continuous loop to handle incoming events == connections.
 * It waits for events with the EpollWrapper.
 * If a vector of events is returned it processes all of them via handleEvent().
 * When events are processed or the waitForEvents() timeout happens checks for connection timeouts with
 * checkForTimeout(). Then cleans up closed connections with cleanupClosedConnections().
 *
 * @throws std::runtime_error if waitForEvents() encounters an error.
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

/* ====== GETTERS ====== */

/**
 * @brief Const Getter for virtual servers.
 *
 * @return const std::map<int, Socket>& Map of virtual servers.
 */
const std::map<int, Socket>& Server::getVirtualServers() const { return m_virtualServers; }

/**
 * @brief Getter for connections.
 *
 * @return std::map<int, Connection>& Map of connections.
 */
std::map<int, Connection>& Server::getConnections() { return m_connections; }

/**
 * @brief Const Getter for connections.
 *
 * @return const std::map<int, Connection>& Map of connections.
 */
const std::map<int, Connection>& Server::getConnections() const { return m_connections; }

/**
 * @brief Const Getter for server configs.
 *
 * Returns the vector of server configs from the configuration file.
 *
 * @return const std::vector<ServerConfig>& Vector of server configs.
 */
const std::vector<ServerConfig>& Server::getServerConfigs() const { return m_configFile.serverConfigs; }

/**
 * @brief Const Getter for client timeout.
 *
 * @return time_t Client timeout in seconds.
 */
time_t Server::getClientTimeout() const { return m_clientTimeout; }

/* ====== SETTERS ====== */

/**
 * @brief Register a virtual server.
 *
 * Registers a server fd with addEvent().
 * If it fails to add the event, it logs an error and closes the server socket.
 * If the server is successfully registered, adds the server to map m_virtualServers.
 * The []-operator creates a new entry in the map if the key does not exist or overwrites an existing one.
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
 * Registers a client fd with addEvent().
 * If it fails to add the event, it logs an error and closes the client socket.
 * If the connection is successfully registered, adds the connection to map m_connections.
 * The []-operator creates a new entry in the map if the key does not exist or overwrites an existing one.
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

/**
 * @brief Setter for client timeout.
 *
 * @param clientTimeout The client timeout in seconds.
 */
void Server::setClientTimeout(time_t clientTimeout) { m_clientTimeout = clientTimeout; }

/* ====== DISPATCH TO EPOLLWRAPPER ====== */

/**
 * @brief Wrapper function to EpollWrapper::addEvent.
 *
 * @param newfd The file descriptor of the new event.
 * @param eventMask The eventmask of the new event to add.
 * @return true if the event was successfully added, false otherwise.
 */
bool Server::addEvent(int newfd, uint32_t eventMask) const { return m_epollWrapper.addEvent(newfd, eventMask); }

/**
 * @brief Wrapper function to EpollWrapper::modifyEvent.
 *
 * @param modfd The file descriptor of the event to modify.
 * @param eventMask The eventmask of the modified event.
 * @return true if the event was successfully modified, false otherwise.
 */
bool Server::modifyEvent(int modfd, uint32_t eventMask) const { return m_epollWrapper.modifyEvent(modfd, eventMask); }

/**
 * @brief Wrapper function to EpollWrapper::removeEvent.
 *
 * @param delfd The file descriptor of the event to remove.
 */
void Server::removeEvent(int delfd) const { m_epollWrapper.removeEvent(delfd); }

/* ====== DISPATCH TO SOCKETPOLICY ====== */

/**
 * @brief Wrapper function to SocketPolicy::resolveListeningAddresses.
 *
 * @param host The host address.
 * @param port The port number.
 * @return struct addrinfo* A pointer to the address information.
 */
struct addrinfo* Server::resolveListeningAddresses(const std::string& host, const std::string& port) const
{
	return m_socketPolicy.resolveListeningAddresses(host, port);
}

/**
 * @brief Wrapper function to SocketPolicy::createListeningSocket.
 *
 * @param addrinfo The address information.
 * @param backlog The maximum length to which the queue of pending connections may grow.
 * @return int The file descriptor of the listening socket.
 */
int Server::createListeningSocket(const struct addrinfo& addrinfo, int backlog) const
{
	return m_socketPolicy.createListeningSocket(addrinfo, backlog);
}

/**
 * @brief Wrapper function to SocketPolicy::retrieveSocketInfo.
 *
 * @param sockaddr The socket address.
 * @param socklen The length of the socket address.
 * @return Socket The socket information.
 */
Socket Server::retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const
{
	return m_socketPolicy.retrieveSocketInfo(sockaddr, socklen);
}

/**
 * @brief Wrapper function to SocketPolicy::acceptSingleConnection().
 *
 * @param sockfd The file descriptor of the socket.
 * @param addr The socket address.
 * @param addrlen The length of the socket address.
 * @return int The file descriptor of the accepted socket.
 */
int Server::acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const
{
	return m_socketPolicy.acceptSingleConnection(sockfd, addr, addrlen);
}

/**
 * @brief Wrapper function to SocketPolicy::readFromSocket.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to read into.
 * @param size The size of the buffer.
 * @param flags The flags for the read operation.
 * @return ssize_t The number of bytes read.
 */
ssize_t Server::readFromSocket(int sockfd, char* buffer, size_t size, int flags) const
{
	return m_socketPolicy.readFromSocket(sockfd, buffer, size, flags);
}

/**
 * @brief Wrapper function to SocketPolicy::writeToSocket.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to write from.
 * @param size The size of the buffer.
 * @param flags The flags for the write operation.
 * @return ssize_t The number of bytes written.
 */
ssize_t Server::writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const
{
	return m_socketPolicy.writeToSocket(sockfd, buffer, size, flags);
}

/* ====== DISPATCH TO REQUESTPARSER ====== */

/**
 * @brief Wrapper function to RequestParser::parseHttpRequest.
 *
 * @param requestString The request string to parse.
 * @param request The HTTPRequest object to store the parsed request.
 */
void Server::parseHttpRequest(const std::string& requestString, HTTPRequest& request)
{
	m_requestParser.parseHttpRequest(requestString, request);
}

/**
 * @brief Wrapper function to RequestParser::clearParser.
 */
void Server::clearParser() { m_requestParser.clearParser(); }

/* ====== DISPATCH TO RESPONSEBUILDER ====== */

/**
 * @brief Wrapper function to ResponseBuilder::buildResponse.
 *
 * @param request The HTTPRequest object to build the response for.
 */
void Server::buildResponse(const HTTPRequest& request) { m_responseBuilder.buildResponse(request); }

/**
 * @brief Wrapper function to ResponseBuilder::getResponse.
 *
 * @return std::string The response string.
 */
std::string Server::getResponse() { return m_responseBuilder.getResponse(); }

/* ====== INITIALIZATION ====== */

/**
 * @brief Initialize virtual servers.
 *
 * Initializes virtual servers by iterating through the serverConfigs
 * vector in the configuration file and opening a socket for each virtual server.
 * It checks for duplicate servers using isDuplicateServer() and
 * skips opening the socket server if it already exists.
 *
 * @param server The server object to initialize virtual servers for.
 * @param backlog The maximum length to which the queue of pending connections for the virtual server may grow.
 * @param serverConfigs The vector of server configurations from the configuration file.
 *
 * @return true if at least one virtual server was added, false otherwise.
 */
bool initVirtualServers(Server& server, int backlog, const std::vector<ServerConfig>& serverConfigs)
{
	LOG_INFO << "Initializing virtual servers";

	for (std::vector<ServerConfig>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {

		LOG_DEBUG << "Adding virtual server: " << iter->serverName << " on " << iter->host << ":" << iter->port;

		if (isDuplicateServer(server, iter->host, webutils::toString(iter->port)))
			continue;

		if (!createVirtualServer(server, iter->host, backlog, webutils::toString(iter->port)))
			LOG_DEBUG << "Failed to add virtual server: " << iter->serverName;
	}

	if (server.getVirtualServers().empty())
		return false;

	LOG_INFO << "Finished setting up virtual servers";
	return true;
}

/**
 * @brief Check for duplicate virtual servers.
 *
 * Checks if a virtual server with the provided host and port already exists in the virtual servers map.
 * If the host is "localhost", it checks for a virtual server with the host address "127.0.0.1" or "::1".
 *
 * @param server The server object to check for duplicate virtual servers.
 * @param host The host address of the virtual server.
 * @param port The port number of the virtual server.
 *
 * @return true if a duplicate virtual server exists, false otherwise.
 */
bool isDuplicateServer(const Server& server, const std::string& host, const std::string& port)
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

/**
 * @brief Creates a new virtual server and registers it with the server object.
 *
 * Creates a listening socket for a virtual server using the provided host, port, and backlog and adds it to the
 * virtual servers map.
 * Uses Server::resolveListeningAddresses() to get a list of address information for the provided host and port.
 * It then tries to create a listening socket for each value in the list using Server::createListeningSocket(). If
 * succesful calls Server::retrieveSocketInfo() and registers the virtual server using Server::registerVirtualServer().
 * If any step fails, it continues to the next value in the list. After the loop, the created address list is freed. If
 * no valid socket was created, it logs an error and returns false.
 *
 * @param server The server object to add the virtual server to.
 * @param host host address for the virtual server.
 * @param backlog maximum length to which the queue of pending connections for the virtual server may grow.
 * @param port port number for the virtual server.
 *
 * @return true if the virtual server was successfully added, false otherwise.
 */
bool createVirtualServer(Server& server, const std::string& host, int backlog, const std::string& port)
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

/* ====== EVENT HANDLING ====== */

/**
 * @brief Accepts new connections or handles existing ones

 * This function acts as a general dispatcher for handling events.
 * Checks if the event file descriptor is in the virtual servers map. If it is, it calls acceptConnections().
 * If the event mask contains EPOLLERR, it logs an error and closes the virtual server.
 *
 * If it is not in the virtual servers map, then it is a connection event.
 * If the event mask contains EPOLLERR or EPOLLHUP, the event mask is set to EPOLLIN.
 * This is done to handle errors and hangups that may occur during the processing of the connection.
 * The Server could then try to recv(), which will return 0 in case of EPOLLHUP or -1 in case of EPOLLERR,
 * resulting in a connection close.
 * @sa https://stackoverflow.com/a/29206631
 *
 * handleConnections() is called with the connection from the connections map.
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
			close(event.data.fd);
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
 * If the event mask does not contain `EPOLLIN`, it logs an error and returns. A server socket should always receive a
 * read event.
 *
 * In a loop, calls Server::acceptSingleConnection() to accept new connections on the server socket.
 * When Server::acceptSingleConnection() is successful, information of the client socket is retrieved with
 * Server::retrieveSocketInfo(). This new client socket is then registered with Server::registerConnection().
 *
 * The while-loop allows for multiple connections to be accepted in one call of this function. It is broken when
 * accept() sets errno to EAGAIN or EWOULDBLOCK (equivalent error codes indicating that a non-blocking operation would
 * normally block), meaning that no more connections are pending. If another errno is returned logs an error and
 * continues to the next iteration of the loop. This is because the server sockets are in EPOLLET mode. If only one
 * connection would be accepted the same server fd would not be reported again.
 *
 * @param server The server object to accept connections for.
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

		const int clientFd = server.acceptSingleConnection(serverFd, addrCast, &clientLen);
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
 * @brief Handle a client connection.
 *
 * After epoll reports an event on a client socket, this function is called to handle the connection. The event from the
 * client can either be incoming (EPOLLIN) or outgoing (EPOLLOUT).
 * This function dispatches to the correct subfunction depending on the connection state:
 * 1. Connection::ReceiveHeader (in): every new connection starts in this state. connectionReceiveHeader() reads the
 * request header from the client and parses it. If the request header is complete, it changes the state to
 * Connection::ReceiveBody or Connection::BuildResponse.
 * 2. Connection::ReceiveBody (in): if the request header indicates a body, connectionReceiveBody() reads the body from
 * the client. If the body is complete, it changes the state to Connection::BuildResponse.
 * 3. Connection::BuildResponse (out): after a complete request (header and optional body) is received
 * connectionBuildResponse() builds the response for the client. It then changes the state to Connection::SendResponse
 * and also calls connectionSendResponse() to try to immediatly send the response to the client.
 * 4. Connection::SendResponse (out): connectionSendResponse() sends the response to the client. This dispatch happens
 * only if the response is not completely sent the first time.
 * 5. Connection::Timeout (out): if the connection has timed out, connectionHandleTimeout() sets the request status to
 * timeout. Then it calls connectionBuildResponse() to build the error message, which then gets sent with
 * connectionSendResponse().
 *
 * @param server The server object to handle the connection for.
 * @param clientFd The file descriptor of the client.
 * @param connection The connection object to handle.
 */
void handleConnection(Server& server, const int clientFd, Connection& connection)
{
	LOG_DEBUG << "Handling connection: " << connection.m_clientSocket << " for server: " << connection.m_serverSocket;

	switch (connection.m_status) {
	case (Connection::ReceiveHeader):
		connectionReceiveHeader(server, clientFd, connection);
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

/**
 * @brief Receive request header from a client.
 *
 * This function reads data from the client socket using Server::readFromSocket().
 * The amount of bytes to read from the socket is determined by the bytes already received from the client. It can never
 * be bigger than the buffer size.
 * - If bytes read is -1, it indicates an internal server error
 * - If bytes read is 0, it indicates that the connection has been closed by the client
 * In both cases the clientFd is closed and the connection status is set to Closed.
 * If bytes read is greater than 0, the bytes received are added to the connection buffer and the bytes received of the
 * connection are updated.
 * If the buffer contains a complete request, it parses the request. Then the request is removed from the client buffer,
 * the connection is set to BuildResponse state and the event is modified to listen to EPOLLOUT.
 * If no complete request was received and the received bytes match the buffer size, sets HTTP status code to 413
 * Request Header Fields Too Large and status to BuildResponse, since the request header was too big.
 *
 * In any case the time since the last event is updated to the current time.
 *
 * @param server The server object which handles the connection.
 * @param clientFd The file descriptor of the client.
 * @param connection The connection object to receive the request for.
 * @todo Implement body handling.
 * @todo make clientHeaderBufferSize configurable.
 */
void connectionReceiveHeader(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "Receive Request Header for: " << connection.m_clientSocket;

	char buffer[Server::s_clientHeaderBufferSize] = {};
	const size_t bytesToRead = Server::s_clientHeaderBufferSize - connection.m_bytesReceived;

	const ssize_t bytesRead = server.readFromSocket(clientFd, buffer, bytesToRead, 0);
	if (bytesRead == -1) {
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
		if (isCompleteRequestHeader(connection.m_buffer)) {
			LOG_DEBUG << "Received complete request header: " << '\n' << connection.m_buffer;
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
			LOG_DEBUG << "Received partial request header: " << '\n' << connection.m_buffer;
			if (connection.m_bytesReceived == Server::s_clientHeaderBufferSize) {
				LOG_ERROR << "Buffer full, didn't receive complete request header from " << connection.m_clientSocket;
				connection.m_request.httpStatus = StatusRequestHeaderFieldsTooLarge;
				connection.m_status = Connection::BuildResponse;
			}
		}
	}
	connection.m_timeSinceLastEvent = std::time(0);
}

/**
 * @brief Checks if the connection buffer contains a request header.
 *
 * A request header is defined as a string that ends with "\r\n\r\n".
 * @param connectionBuffer The buffer to check for a complete request.
 * @return true if the buffer contains a complete request, false otherwise.
 */
bool isCompleteRequestHeader(const std::string& connectionBuffer)
{
	return (connectionBuffer.find("\r\n\r\n") != std::string::npos);
}

void connectionReceiveBody(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "ReceiveBody for: " << connection.m_clientSocket;

	(void)server;
	(void)clientFd;
}

/**
 * @brief Builds the response for a client connection.
 *
 * Builds the response for a client connection by calling Server::buildResponse().
 * The response is stored in the connection buffer.
 * The connection status is set to SendResponse and the response is sent to the client by calling
 * connectionSendResponse().
 *
 * @param server The server object which handles the connection.
 * @param clientFd The file descriptor of the client.
 * @param connection The connection object to build the response for.
 */
void connectionBuildResponse(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "BuildResponse for: " << connection.m_clientSocket;

	server.buildResponse(connection.m_request);
	connection.m_buffer = server.getResponse();
	connection.m_status = Connection::SendResponse;
	connectionSendResponse(server, clientFd, connection);
}

/**
 * @brief Sends the response to a client connection.
 *
 * Sends the response to a client connection by calling Server::writeToSocket().
 * The amount of bytes to send is determined by the size of the connection buffer.
 * If sent bytes == -1 it indicates an internal server error and the connection is closed.
 * If the response is not completely sent, the bytes sent are removed from the buffer and timeSinceLastEvent is
 * updated.
 * If the response is completely sent and the connection is set to close, the client socket is closed and the
 * connection status is set to Closed.
 * If the response is completely sent and the connection is not set to close, the event is modified to listen to
 * EPOLLIN and the connection is cleared.
 * @param server The server object which handles the connection.
 * @param clientFd The file descriptor of the client.
 * @param connection The connection object to send the response for.
 */
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

/**
 * @brief Handles a timeout for a client connection.
 *
 * Handles a timeout for a client connection by setting the request status to StatusRequestTimeout.
 * Then it builds the error message by calling connectionBuildResponse().
 *
 * @param server The server object which handles the connection.
 * @param clientFd The file descriptor of the client.
 * @param connection The connection object to handle the timeout for.
 */
void connectionHandleTimeout(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "Timeout for: " << connection.m_clientSocket;

	connection.m_request.shallCloseConnection = true;
	connection.m_request.httpStatus = StatusRequestTimeout;

	connectionBuildResponse(server, clientFd, connection);
}

/**
 * @brief Iterates through all connections and checks if any have timed out.
 *
 * If the connection status is Closed, it is skipped.
 * The time since last event is saved in a variable to print it to the log and check
 * if it is greater than the timeout set for the server. If it is, the connection is modified to EPOLLOUT,
 * and the state is set to Timeout. When the fd is ready to receive a message the timeout error will be send.
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

/* ====== CLEANUP ====== */

/**
 * @brief Iterates through all connections and removes closed ones.
 *
 * The for loop through the connections map has no increment statement because the
 * iterator is incremented in the loop body. If .erase() is called on an iterator,
 * it is invalidated.
 *
 * @param server The server object to cleanup closed connections for.
 */
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
