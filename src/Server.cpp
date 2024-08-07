#include "Server.hpp"

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
Server::Server(const ConfigFile& configFile, int epollTimeout, size_t maxEvents)
	: m_configFile(configFile)
	, m_epfd(epoll_create(1))
	, m_epollTimeout(epollTimeout)
	, m_epollEvents(maxEvents)
	, m_backlog(s_backlog)
	, m_clientTimeout(s_clientTimeout)
	, m_responseBuilder(m_configFile, m_fileSystemPolicy)
{
	if (m_epfd < 0)
		throw std::runtime_error("epoll_create:" + std::string(strerror(errno)));
	if (!initVirtualServers())
		throw std::runtime_error("Failed to initialize virtual servers");
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
 */
Server::~Server()
{
	for (std::map<int, Socket>::iterator iter = m_virtualServers.begin(); iter != m_virtualServers.end(); ++iter)
		close(iter->first);

	for (std::map<int, Connection>::iterator iter = m_connections.begin(); iter != m_connections.end(); ++iter) {
		removeEvent(m_epfd, iter->first);
		iter->second.closeConnection();
	}
	close(m_epfd);
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
		const int nfds = waitForEvents(m_epfd, m_epollEvents, m_epollTimeout);

		for (std::vector<struct epoll_event>::const_iterator iter = m_epollEvents.begin();
			 iter != m_epollEvents.begin() + nfds; ++iter) {
			handleEvent(*iter);
		}
		handleTimeout();
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
bool Server::initVirtualServers()
{
	LOG_INFO << "Initializing virtual servers";

	for (std::vector<ServerConfig>::const_iterator iter = m_configFile.serverConfigs.begin();
		 iter != m_configFile.serverConfigs.end(); ++iter) {

		LOG_DEBUG << "Adding virtual server: " << iter->serverName << " on " << iter->host << ":" << iter->port;

		if (checkDuplicateServer(m_virtualServers, iter->host, webutils::toString(iter->port)))
			continue;

		if (!addVirtualServer(iter->host, m_backlog, webutils::toString(iter->port))) {
			LOG_DEBUG << "Failed to add virtual server: " << iter->serverName;
			continue;
		}
	}

	if (m_virtualServers.empty())
		return false;

	LOG_INFO << "Finished setting up virtual servers";
	return true;
}

/**
 * @brief Add a virtual server.
 *
 * Creates a listening socket for a virtual server using the provided host, port, and backlog.
 * It uses getaddrinfo() to get the address information for the provided host and port.
 * It then creates a listening socket using createListeningSocket() and registers the virtual server
 * using Server::registerVirtualServer().
 *
 * @param host host address for the virtual server.
 * @param backlog maximum length to which the queue of pending connections for the virtual server may grow.
 * @param port port number for the virtual server.
 *
 * @return true if the virtual server was successfully added, false otherwise.
 */
bool Server::addVirtualServer(const std::string& host, const int backlog, const std::string& port)
{
	const char* node = NULL;

	if (!host.empty() || host != "*")
		node = host.c_str();

	struct addrinfo hints = {
		AI_PASSIVE, /* .ai_flags - If node is NULL returned address will be suitable to bind(2) a socket which can
					   accept(2) connections.*/
		AF_UNSPEC, /*.ai_family - Allow IPv4 or IPv6 */
		SOCK_STREAM, /* .ai_socktype - TCP uses SOCK_STREAM */
		0, /* .ai_protocol - Accept any protocoll */
		sizeof(struct addrinfo), /* .ai_addrlen - not used */
		NULL, /* .ai_addr - not used */
		NULL, /* .ai_canonname - not used */
		NULL /* .ai_next - not used */
	};

	struct addrinfo* list = NULL;
	const int result = getaddrinfo(node, port.c_str(), &hints, &list);
	if (result != 0) {
		LOG_ERROR << "getaddrinfo(): " << gai_strerror(result);
		return false;
	}

	size_t successfulSock = 0;
	size_t countTryCreateSocket = 1;
	for (struct addrinfo* curr = list; curr != NULL; curr = curr->ai_next) {
		LOG_DEBUG << countTryCreateSocket << ". try to create listening socket";

		const int newFd = createListeningSocket(curr, backlog);
		if (newFd == -1)
			continue;

		const Socket serverSock = retrieveSocketInfo(newFd, curr->ai_addr, curr->ai_addrlen);
		if (serverSock.fd == -1)
			continue;

		if (!registerVirtualServer(serverSock))
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
 * See also https://stackoverflow.com/a/29206631
 *
 * Checks if the event file descriptor is in the virtual servers map. If it is, it calls acceptConnections().
 * If it is not, it calls handleConnections() with the connection from the connections map.
 * A connection should exist in the connections map, therefore it is not checked if the file descriptor
 * is in the connections map.
 *
 * @param event The epoll event to handle.
 */
void Server::handleEvent(struct epoll_event event)
{
	uint32_t eventMask = event.events;
	if ((eventMask & EPOLLERR) != 0) {
		LOG_DEBUG << "epoll_wait: EPOLLERR";
		eventMask = EPOLLIN;
	} else if ((eventMask & EPOLLHUP) != 0) {
		LOG_DEBUG << "epoll_wait: EPOLLHUP";
		eventMask = EPOLLIN;
	}
	std::map<int, Socket>::const_iterator iter = m_virtualServers.find(event.data.fd);
	if (iter != m_virtualServers.end())
		acceptConnections(iter->second, eventMask);
	else
		handleConnections(m_connections.at(event.data.fd));
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
 * @param serverSock Server socket which reported an event.
 * @param eventMask Event mask of the reported event.
 */
void Server::acceptConnections(const Socket& serverSock, uint32_t eventMask)
{
	LOG_DEBUG << "Accept connections on: " << serverSock;

	if ((eventMask & EPOLLERR) != 0) {
		LOG_ERROR << "Error condition happened on the associated file descriptor of " << serverSock;
		close(serverSock.fd);
		m_virtualServers.erase(serverSock.fd);
		return;
	}

	if ((eventMask & EPOLLIN) == 0) {
		LOG_ERROR << "Received unknown event:" << eventMask;
		return;
	}

	while (true) {
		struct sockaddr_storage clientAddr = {};
		socklen_t clientLen = sizeof(clientAddr);

		// NOLINTNEXTLINE: we need to use reinterpret_cast to convert sockaddr_storage to sockaddr
		struct sockaddr* addrCast = reinterpret_cast<struct sockaddr*>(&clientAddr);

		const int clientFd = accept(serverSock.fd, addrCast, &clientLen);
		if (clientFd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return; // No more pending connections
			LOG_ERROR << "accept(): " << strerror(errno);
			continue;
		}

		const Socket clientSock = retrieveSocketInfo(clientFd, addrCast, clientLen);
		if (clientSock.fd == -1)
			continue;

		if (!registerConnection(serverSock, clientSock))
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
void Server::handleConnections(const Connection& connection)
{
	LOG_DEBUG << "Handling connection: " << connection.getClientSocket()
			  << " for server: " << connection.getServerSocket();
	// Handle client data
	char buffer[s_bufferSize];
	HTTPRequest request;

	request.method = MethodCount;
	request.httpStatus = StatusOK;
	request.shallCloseConnection = false;

	const ssize_t bytesRead = recv(connection.getClientSocket().fd, buffer, s_bufferSize, 0);
	if (bytesRead < 0) {
		std::cerr << "error: read\n";
		close(connection.getClientSocket().fd);
	} else if (bytesRead == 0) {
		// Connection closed by client
		close(connection.getClientSocket().fd);
	} else {
		m_connectionBuffers[connection.getClientSocket().fd] += buffer;
		if (checkForCompleteRequest(connection.getClientSocket().fd)) {
			LOG_DEBUG << "Received complete request: " << '\n' << m_connectionBuffers[connection.getClientSocket().fd];
			try {
				m_requestParser.parseHttpRequest(m_connectionBuffers[connection.getClientSocket().fd], request);
				m_requestParser.clearParser();
			} catch (std::exception& e) {
				LOG_ERROR << "Error: " << e.what();
			}
			m_responseBuilder.buildResponse(request);
			send(connection.getClientSocket().fd, m_responseBuilder.getResponse().c_str(),
				m_responseBuilder.getResponse().size(), 0);
		} else {
			LOG_DEBUG << "Received partial request: " << '\n' << m_connectionBuffers[connection.getClientSocket().fd];
		}
	}
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
 */
void Server::handleTimeout()
{
	for (std::map<int, Connection>::iterator iter = m_connections.begin(); iter != m_connections.end();
		/* no increment */) {
		time_t timeSinceLastEvent = iter->second.getTimeSinceLastEvent();
		LOG_DEBUG << iter->second.getClientSocket() << ": Time since last event: " << timeSinceLastEvent;
		if (timeSinceLastEvent > m_clientTimeout) {
			LOG_INFO << "Connection timeout: " << iter->second.getClientSocket();
			removeEvent(m_epfd, iter->first);
			iter->second.closeConnection();
			m_connections.erase(iter++);
		} else
			++iter;
	}
}

bool Server::checkForCompleteRequest(int clientSock)
{
	const size_t headerEndPos = m_connectionBuffers[clientSock].find("\r\n\r\n");

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
bool Server::registerVirtualServer(const Socket& serverSock)
{
	struct epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = serverSock.fd;

	if (!addEvent(m_epfd, serverSock.fd, &event)) {
		close(serverSock.fd);
		LOG_ERROR << "Failed to add event for " << serverSock;
		return false;
	}

	m_virtualServers[serverSock.fd] = serverSock;

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
bool Server::registerConnection(const Socket& serverSock, const Socket& clientSock)
{
	struct epoll_event event = {};
	event.events = EPOLLIN;
	event.data.fd = clientSock.fd;

	if (!addEvent(m_epfd, clientSock.fd, &event)) {
		close(clientSock.fd);
		LOG_ERROR << "Failed to add event for " << clientSock;
		return false;
	}

	m_connections[clientSock.fd] = Connection(serverSock, clientSock);
	m_connectionBuffers[clientSock.fd] = "";

	LOG_INFO << "New Connection: " << clientSock << " for server: " << serverSock;
	return true;
}

/* ====== HELPER FUNCTIONS ====== */

/**
 * @brief Create a listening socket.
 *
 * Creates a new socket with the provided address information and binds it to the address.
 * It sets the socket to non-blocking mode and listens for incoming connections.
 *
 * @param addrinfo The address information for the socket.
 * @param backlog The maximum length to which the queue of pending connections may grow.
 *
 * @return The file descriptor of the new socket if successful, -1 otherwise.
 */
int createListeningSocket(struct addrinfo* addrinfo, int backlog)
{
	addrinfo->ai_socktype |= SOCK_NONBLOCK;

	const int newFd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
	if (newFd == -1) {
		LOG_DEBUG << "socket(): " << strerror(errno);
		return -1;
	}

	int reuse = 1;
	if (-1 == setsockopt(newFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) {
		close(newFd);
		LOG_DEBUG << "setsockopt(): " << strerror(errno);
		return -1;
	}

	if (-1 == bind(newFd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
		close(newFd);
		LOG_DEBUG << "bind(): " << strerror(errno);
		return -1;
	}

	if (-1 == listen(newFd, backlog)) {
		close(newFd);
		LOG_DEBUG << "listen(): " << strerror(errno);
		return -1;
	}
	return newFd;
}

/**
 * @brief Retrieve socket information.
 *
 * Retrieves the host and port information from a socket and returns it as a Socket object.
 *
 * @param sockFd The file descriptor of the socket.
 * @param sockaddr The socket address structure.
 * @param socklen The length of the socket address structure.
 *
 * @return A Socket object containing the host and port information.
 */
Socket retrieveSocketInfo(const int sockFd, const struct sockaddr* sockaddr, socklen_t socklen)
{
	char bufferHost[NI_MAXHOST];
	char bufferPort[NI_MAXSERV];

	const int ret = getnameinfo(
		sockaddr, socklen, bufferHost, NI_MAXHOST, bufferPort, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret != 0) {
		LOG_ERROR << "getnameinfo(): " << gai_strerror(ret);
		close(sockFd);
		const Socket errSock = { -1, "", "" };
		return (errSock);
	}

	const Socket newSock = { sockFd, bufferHost, bufferPort };

	return (newSock);
}

/**
 * @brief Wait for events on an epoll instance.
 *
 * If epoll_wait returns -1, it checks if the error is due to an interrupt signal.
 * If it is, it returns 0. Otherwise, it throws a runtime error with the error message.
 * If epoll_wait returns 0, it logs a timeout message. Otherwise, it logs the number of events.
 *
 * @param epfd The file descriptor of the epoll instance.
 * @param events The vector to store the events.
 * @param timeout The timeout value in milliseconds.
 *
 * @return The number of events that occurred.

 * @throws std::runtime_error if epoll_wait encounters an error.
 */
int waitForEvents(int epfd, std::vector<struct epoll_event>& events, int timeout)
{
	LOG_DEBUG << "Waiting for events";

	const int nfds = epoll_wait(epfd, &events[0], static_cast<int>(events.size()), timeout);
	if (nfds == -1) {
		if (errno == EINTR)
			return 0;
		throw std::runtime_error("epoll_wait:" + std::string(strerror(errno)));
	}

	if (nfds == 0)
		LOG_DEBUG << "epoll_wait: Timeout";
	else
		LOG_DEBUG << "epoll_wait: " << nfds << " events";
	return nfds;
}

/**
 * @brief Add an event to an epoll instance.
 *
 * If epoll_ctl returns -1, it logs an error message and returns false.
 *
 * @param epfd The file descriptor of the epoll instance.
 * @param newfd The file descriptor of the new event.
 * @param event The event to add.
 *
 * @return true if the event was successfully added, false otherwise.
 */
bool addEvent(int epfd, int newfd, epoll_event* event)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_ADD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Added new fd: " << newfd;

	return true;
}

/**
 * @brief Remove an event from an epoll instance.
 *
 * If epoll_ctl returns -1, it logs an error message.
 *
 * @param epfd The file descriptor of the epoll instance.
 * @param delfd The file descriptor of the event to remove.
 */
void removeEvent(int epfd, int delfd)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, delfd, NULL)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno);
		return;
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd;
}

/**
 * @brief Modify an event in an epoll instance.
 *
 * If epoll_ctl returns -1, it logs an error message and returns false.
 *
 * @param epfd The file descriptor of the epoll instance.
 * @param modfd The file descriptor of the event to modify.
 * @param event The modified event.
 *
 * @return true if the event was successfully modified, false otherwise.
 */
bool modifyEvent(int epfd, int modfd, epoll_event* event)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, modfd, event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_MOD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Modified fd: " << modfd;
	return true;
}

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
bool checkDuplicateServer(const std::map<int, Socket>& virtualServers, const std::string& host, const std::string& port)
{
	if (host == "localhost") {
		for (std::map<int, Socket>::const_iterator iter = virtualServers.begin(); iter != virtualServers.end();
			 ++iter) {
			if ((iter->second.host == "127.0.0.1" || iter->second.host == "::1") && iter->second.port == port) {
				LOG_DEBUG << "Virtual server already exists: " << iter->second;
				return true;
			}
		}
	} else {
		for (std::map<int, Socket>::const_iterator iter = virtualServers.begin(); iter != virtualServers.end();
			 ++iter) {
			if (iter->second.host == host && iter->second.port == port) {
				LOG_DEBUG << "Virtual server already exists: " << iter->second;
				return true;
			}
		}
	}
	return false;
}
