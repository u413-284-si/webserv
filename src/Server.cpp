#include "Server.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.

 * The Server constructor initializes a Server object with the provided configuration file, EpollWrapper, and
 * SocketOps, along with other member variables.
 * The passed EpollWrapper is saved as a non-const ref as it needs to be modifiable.
 * The passed SocketOps is saved as a const ref.
 *
 * @param configFile The `configFile` parameter is an object of type `ConfigFile` that is passed to the
 * `Server` constructor. It is used to configure the server with settings such especially the number and
 * configuration of virtual servers.
 * @param epollWrapper A ready to use epoll instance. Can be mocked for testing.
 * @param fileSystemOps Wrapper for filesystem-related functions. Can be mocked for testing.
 * @param socketOps Wrapper for socket-related functions. Can be mocked for testing.
 * @param processOps Wrapper for process-related functions. Can be mocked for testing.

 * @todo Several variables are init to static ones, could be passed as parameters or set in config file.
 */
Server::Server(const ConfigFile& configFile, EpollWrapper& epollWrapper, const FileSystemOps& fileSystemOps,
	const SocketOps& socketOps, const ProcessOps& processOps)
	: m_configFile(configFile)
	, m_epollWrapper(epollWrapper)
	, m_fileSystemOps(fileSystemOps)
	, m_socketOps(socketOps)
	, m_processOps(processOps)
	, m_backlog(s_backlog)
	, m_clientTimeout(s_clientTimeout)
	, m_responseBuilder(m_fileSystemOps)
	, m_targetResourceHandler(m_fileSystemOps)
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
 * 3. Closes all open CGI connections to release associated resources.
 * @todo Is it possible to send a last message to all connected clients to notify server shutdown?
 */
Server::~Server()
{
	for (std::map<int, Socket>::iterator iter = m_virtualServers.begin(); iter != m_virtualServers.end(); ++iter) {
		m_epollWrapper.removeEvent(iter->first);
		close(iter->first);
	}

	for (std::map<int, Connection>::iterator iter = m_connections.begin(); iter != m_connections.end(); ++iter) {
		m_epollWrapper.removeEvent(iter->first);
		close(iter->first);
	}

	for (std::map<int, Connection*>::iterator iter = m_cgiConnections.begin(); iter != m_cgiConnections.end(); ++iter) {
		m_epollWrapper.removeEvent(iter->first);
		close(iter->first);
	}
}

/* ====== GETTERS ====== */

/**
 * @brief Getter for virtual servers.
 *
 * @return std::map<int, Socket>& Map of virtual servers.
 */
std::map<int, Socket>& Server::getVirtualServers() { return m_virtualServers; }

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
 * @brief Getter for cgiConnections.
 *
 * @return std::map<int, Connection&>& Map of cgiConnections.
 */
std::map<int, Connection*>& Server::getCGIConnections() { return m_cgiConnections; }

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
const std::vector<ConfigServer>& Server::getServerConfigs() const { return m_configFile.servers; }

/**
 * @brief Const Getter for client timeout.
 *
 * @return time_t Client timeout in seconds.
 */
time_t Server::getClientTimeout() const { return m_clientTimeout; }

/**
 * @brief Const Getter for ProcessOps.
 *
 * @return const ProcessOps& Registered ProcessOps object.
 */
const ProcessOps& Server::getProcessOps() const { return m_processOps; }

/**
 * @brief Const Getter for FileSystemOps.
 *
 * @return const FileSystemOps& Registered FileSystemOps object.
 */
const FileSystemOps& Server::getFileSystemOps() const { return m_fileSystemOps; }

/**
 * @brief Getter for internal buffer.
 *
 * @return std::vector<char>& buffer.
 */
std::vector<char>& Server::getBuffer() { return m_buffer; }

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
 * If the client fd is successfully registered, creates a new struct Connection with serverSock and clientSock.
 * Then adds the Connection to map m_connections via std::map::insert().
 * If it couldn't insert the connection, it logs an error and closes the client socket.
 *
 * @param serverSock The socket of the server that the connection is associated with.
 * @param clientFd The file descriptor of the client connection to register.
 * @param clientSock The socket of the client connection to register.
 * @return true if the connection was successfully registered, false otherwise.
 */
bool Server::registerConnection(const Socket& serverSock, int clientFd, const Socket& clientSock)
{
	if (!m_epollWrapper.addEvent(clientFd, EPOLLIN)) {
		close(clientFd);
		LOG_ERROR << "Failed to add event for " << clientSock;
		return false;
	}

	Connection newConnection(serverSock, clientSock, clientFd, m_configFile.servers);
	if (newConnection.m_status == Connection::Closed) {
		close(clientFd);
		LOG_ERROR << "Failed to set active server for " << clientSock;
		return false;
	}

	std::pair<std::map<int, Connection>::iterator, bool> ret
		= m_connections.insert(std::pair<int, Connection>(clientFd, newConnection));
	if (!ret.second) {
		close(clientFd);
		LOG_ERROR << "Failed to add connection for " << clientSock << ": it already exists";
		return false;
	}

	LOG_INFO << "New Connection: " << clientSock << " for server: " << serverSock;
	return true;
}

/**
 * @brief Register a CGI file descriptor.
 *
 * Registers a pipe fd with addEvent().
 * If it fails to add the event, it logs an error and closes the pipe.
 * If the fd is successfully registered, adds it togther with the belonging
 * connection to map m_connections.
 *
 * @param pipeFd The file descriptor of the pipe to register.
 * @param eventMask The event mask to register the pipe with.
 * @param connection The connection to register the pipe for.
 *
 * @return true if the CGI file descriptor was successfully registered, false otherwise.
 */
bool Server::registerCGIFileDescriptor(int pipeFd, uint32_t eventMask, Connection& connection)
{
	if (!m_epollWrapper.addEvent(pipeFd, eventMask)) {
		webutils::closeFd(pipeFd);
		LOG_ERROR << "Failed to add event for " << pipeFd;
		return false;
	}

	std::pair<std::map<int, Connection*>::iterator, bool> ret
		= m_cgiConnections.insert(std::pair<int, Connection*>(pipeFd, &connection));
	if (!ret.second) {
		webutils::closeFd(pipeFd);
		LOG_ERROR << "Failed to add connection for " << pipeFd << ": it already exists";
		return false;
	}

	LOG_DEBUG << "New CGI File Descriptor: " << pipeFd << " registered for " << connection.m_clientSocket;
	return true;
}

/**
 * @brief Removes a virtual server.
 *
 * @param delfd file descriptor of the server to be removed.
 */
void Server::removeVirtualServer(int delfd)
{
	removeEvent(delfd);
	getVirtualServers().erase(delfd);
	close(delfd);
	LOG_DEBUG << "Removed virtual server with FD: " << delfd;
}

/**
 * @brief Removes a Connection from the Server.
 *
 * @param delFd File descriptor to be removed.
 */
void Server::removeConnection(int delFd)
{
	const Socket clientSocket = getConnections().at(delFd).m_clientSocket;
	removeEvent(delFd);
	getConnections().erase(delFd);
	close(delFd);
	LOG_DEBUG << "Removed Connection: " << clientSocket << " on fd: " << delFd;
}

/**
 * @brief Removes a CGI file descriptor from the server.
 *
 * This function removes a CGI file descriptor from the server by performing the following steps:
 * 1. Removes the event associated with the file descriptor in the epoll instance.
 * 2. Erases the file descriptor from the CGI connections map.
 * 3. Closes the file descriptor.
 * 4. Sets the file descriptor to -1 to indicate it is no longer valid.
 *
 * @param delfd Reference to the file descriptor to be removed.
 */
void Server::removeCGIFileDescriptor(int& delfd)
{
	this->removeEvent(delfd);
	this->getCGIConnections().erase(delfd);
	LOG_DEBUG << "CGI File Descriptor: " << delfd << " removed from server";
	webutils::closeFd(delfd);
}

/**
 * @brief Setter for client timeout.
 *
 * @param clientTimeout The client timeout in seconds.
 */
void Server::setClientTimeout(time_t clientTimeout) { m_clientTimeout = clientTimeout; }

/* ====== DISPATCH TO EPOLLWRAPPER ====== */

/**
 * @brief Wrapper function to EpollWrapper::waitForEvents.
 *
 * @return int The number of events that were processed.
 * @throws std::runtime_error if an error occurs during the epoll_wait call.
 */
int Server::waitForEvents() { return m_epollWrapper.waitForEvents(); }

/**
 * @brief Wrapper function to EpollWrapper::eventsBegin.
 *
 * @return std::vector<struct epoll_event>::const_iterator An iterator to the beginning of the events vector.
 */
std::vector<struct epoll_event>::const_iterator Server::eventsBegin() const { return m_epollWrapper.eventsBegin(); }

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

/**
 * @brief Wrapper function to EpollWrapper::getEpollFd.
 *
 * @return int The file descriptor of the epoll instance.
 */
int Server::getEpollFd() const { return m_epollWrapper.getEpollFd(); }

/* ====== DISPATCH TO SOCKETOPS ====== */

/**
 * @brief Wrapper function to SocketOps::resolveListeningAddresses.
 *
 * @param host The host address.
 * @param port The port number.
 * @return struct addrinfo* A pointer to the address information.
 */
struct addrinfo* Server::resolveListeningAddresses(const std::string& host, const std::string& port) const
{
	return m_socketOps.resolveListeningAddresses(host, port);
}

/**
 * @brief Wrapper function to SocketOps::createListeningSocket.
 *
 * @param addrinfo The address information.
 * @param backlog The maximum length to which the queue of pending connections may grow.
 * @return int The file descriptor of the listening socket.
 */
int Server::createListeningSocket(const struct addrinfo* addrinfo, int backlog) const
{
	assert(addrinfo != NULL);

	return m_socketOps.createListeningSocket(addrinfo, backlog);
}

/**
 * @brief Wrapper function to SocketOps::retrieveSocketInfo.
 *
 * @param sockaddr The socket address.
 * @return Socket The socket information.
 */
Socket Server::retrieveSocketInfo(struct sockaddr* sockaddr) const
{
	assert(sockaddr != NULL);

	return m_socketOps.retrieveSocketInfo(sockaddr);
}

/**
 * @brief Wrapper function to SocketOps::retrieveBoundSocketInfo.
 *
 * @param sockfd The socket fd which is bound to a socket.
 * @return Socket The socket information of the bound socket.
 */
Socket Server::retrieveBoundSocketInfo(int sockfd) const { return m_socketOps.retrieveBoundSocketInfo(sockfd); }

/**
 * @brief Wrapper function to SocketOps::acceptSingleConnection().
 *
 * @param sockfd The file descriptor of the socket.
 * @param addr The socket address.
 * @param addrlen The length of the socket address.
 * @return int The file descriptor of the accepted socket.
 */
int Server::acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const
{
	assert(addr != NULL);
	assert(addrlen != NULL);

	return m_socketOps.acceptSingleConnection(sockfd, addr, addrlen);
}

/**
 * @brief Wrapper function to SocketOps::readFromSocket.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to read into.
 * @param size The size of the buffer.
 * @param flags The flags for the read operation.
 * @return ssize_t The number of bytes read.
 */
ssize_t Server::readFromSocket(int sockfd, char* buffer, size_t size, int flags) const
{
	assert(buffer != NULL);

	return m_socketOps.readFromSocket(sockfd, buffer, size, flags);
}

/**
 * @brief Wrapper function to SocketOps::writeToSocket.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to write from.
 * @param size The size of the buffer.
 * @param flags The flags for the write operation.
 * @return ssize_t The number of bytes written.
 */
ssize_t Server::writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const
{
	assert(buffer != NULL);

	return m_socketOps.writeToSocket(sockfd, buffer, size, flags);
}

/* ====== DISPATCH TO PROCESSOPS ====== */

/**
 * @brief Wrapper function to ProcessOps::readProcess.
 *
 * This function delegates the read operation to the m_processOps object, which
 * handles the actual reading process.
 *
 * @param fileDescriptor The file descriptor from which to read data.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param size The number of bytes to read from the file descriptor.
 * @return ssize_t The number of bytes read, or -1 if an error occurs.
 */
ssize_t Server::readProcess(int fileDescriptor, char* buffer, size_t size) const
{
	return m_processOps.readProcess(fileDescriptor, buffer, size);
}

/**
 * @brief Wrapper function to ProcessOps::writeProcess.
 *
 * This function delegates the write operation to the process operations handler.
 *
 * @param fileDescriptor The file descriptor to write to.
 * @param buffer The buffer containing the data to be written.
 * @param size The number of bytes to write from the buffer.
 * @return ssize_t The number of bytes written, or -1 on error.
 */
ssize_t Server::writeProcess(int fileDescriptor, const char* buffer, size_t size) const
{
	return m_processOps.writeProcess(fileDescriptor, buffer, size);
}

/**
 * @brief Wrapper function to ProcessOps::waitForProcess.
 *
 * @param pid Pid of the child process to wait for.
 * @param wstatus Stores status information which can be inspected with macros.
 * @param options Options to influence behavior.
 * @return pid_t Process ID of child whose state has changed, or -1 on failure.
 */
pid_t Server::waitForProcess(pid_t pid, int* wstatus, int options) const
{
	return m_processOps.waitForProcess(pid, wstatus, options);
}

/* ====== DISPATCH TO REQUESTPARSER ====== */

/**
 * @brief Wrapper function to RequestParser::parseHeader.
 *
 * @param requestString The request string to parse.
 * @param request The HTTPRequest object to store the parsed request.
 */
void Server::parseHeader(const std::string& requestString, HTTPRequest& request)
{
	m_requestParser.parseHeader(requestString, request);
}

/**
 * @brief Wrapper function to RequestParser::parseBody.
 *
 * @param request The HTTPRequest object to store the parsed body.
 */
void Server::parseChunkedBody(std::string& bodyBuffer, HTTPRequest& request)
{
	RequestParser::parseChunkedBody(bodyBuffer, request);
}

/**
 * @brief Wrapper function to RequestParser::decodeMultipartFormdata.
 *
 * @param request The HTTP request containing the multipart/form-data content to decode.
 */
void Server::decodeMultipartFormdata(HTTPRequest& request) { m_requestParser.decodeMultipartFormdata(request); }

/* ====== DISPATCH TO RESPONSEBUILDER ====== */

/**
 * @brief Wrapper function to ResponseBuilder::buildResponse.
 *
 * @param connection The Connection to build the response for.
 */
void Server::buildResponse(Connection& connection) { m_responseBuilder.buildResponse(connection); }

/**
 * @brief Wrapper function to ResponseBuilder::getResponse.
 *
 * @return std::string The response string.
 */
std::string Server::getResponse() { return m_responseBuilder.getResponse(); }

/* ====== DISPATCH TO TARGETRESOURCEHANDLER ====== */

/**
 * @brief Wrapper function to TargetResourceHandler::execute.
 *
 * @param connection The Connection object to handle the target resource for.
 */
void Server::findTargetResource(Connection& connection)
{
	m_targetResourceHandler.execute(connection.m_request, connection.location, connection.serverConfig);
}

/* ====== INITIALIZATION ====== */

/**
 * @brief Initialize virtual servers.
 *
 * First initializes all wildcard servers (0.0.0.0) in the configuration file by opening a socket for them. They need to
 * be initialized first because any already initialized socket on the same port would block the wildcard server. Then
 * initializes the remaining virtual servers.
 * It checks for duplicate servers using isDuplicateServer() and skips opening the socket server if it already exists.
 *
 * @param server The server object to initialize virtual servers for.
 * @param backlog The maximum length to which the queue of pending connections for the virtual server may grow.
 * @param serverConfigs The vector of server configurations from the configuration file.
 *
 * @return true if at least one virtual server was added, false otherwise.
 */
bool initVirtualServers(Server& server, int backlog, const std::vector<ConfigServer>& serverConfigs)
{
	LOG_INFO << "Initializing virtual servers";

	LOG_DEBUG << "Check for wildcard servers";
	const std::string wildcard = "0.0.0.0";

	for (std::vector<ConfigServer>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {
		if (iter->host == wildcard) {

			LOG_DEBUG << "Adding virtual server: " << iter->host << ":" << iter->port;

			if (isDuplicateServer(server, iter->host, iter->port))
				continue;

			if (!createVirtualServer(server, iter->host, backlog, iter->port))
				LOG_DEBUG << "Failed to add virtual server: " << iter->host << ":" << iter->port;
		}
	}

	LOG_DEBUG << "Add remaining virtual servers";

	for (std::vector<ConfigServer>::const_iterator iter = serverConfigs.begin(); iter != serverConfigs.end(); ++iter) {

		LOG_DEBUG << "Adding virtual server: " << iter->host << ":" << iter->port;

		if (isDuplicateServer(server, iter->host, iter->port))
			continue;

		if (!createVirtualServer(server, iter->host, backlog, iter->port))
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
 * If the existing server is a wildcard server (0.0.0.0) only the port needs to match.
 *
 * @param server The server object to check for duplicate virtual servers.
 * @param host The host address of the virtual server.
 * @param port The port number of the virtual server.
 *
 * @return true if a duplicate virtual server exists, false otherwise.
 */
bool isDuplicateServer(const Server& server, const std::string& host, const std::string& port)
{
	const std::string wildcard = "0.0.0.0";

	if (host == "localhost") {
		for (std::map<int, Socket>::const_iterator iter = server.getVirtualServers().begin();
			 iter != server.getVirtualServers().end(); ++iter) {
			if ((iter->second.host == wildcard || iter->second.host == "127.0.0.1" || iter->second.host == "::1")
				&& iter->second.port == port) {
				LOG_DEBUG << "Virtual server already exists: " << iter->second;
				return true;
			}
		}
	} else {
		for (std::map<int, Socket>::const_iterator iter = server.getVirtualServers().begin();
			 iter != server.getVirtualServers().end(); ++iter) {
			if ((iter->second.host == wildcard || iter->second.host == host) && iter->second.port == port) {
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

		const int newFd = server.createListeningSocket(curr, backlog);
		if (newFd == -1)
			continue;

		const Socket serverSock = server.retrieveSocketInfo(curr->ai_addr);
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
 * @brief Runs the main event loop with a server object.
 *
 * This functions enters a continuous loop to handle incoming events == connections.
 * It waits for events with the EpollWrapper.
 * If a vector of events is returned it processes all of them via handleEvent().
 * When events are processed or the server.waitForEvents() timeout happens checks for connection timeouts with
 * checkForTimeout().
 * The loop continues until a signal is received and saved in g_SignalStatus. It then logs the signal.
 * If the signal is SIGQUIT it performs a graceful shutdown with shutdownServer().
 *
 * @param server The server object to run the event loop for.
 * @throws std::runtime_error if waitForEvents() encounters an error.
 */
void runServer(Server& server)
{
	LOG_INFO << "Server started";

	while (g_signalStatus == 0) {
		const int nfds = server.waitForEvents();

		for (std::vector<struct epoll_event>::const_iterator iter = server.eventsBegin();
			 iter != server.eventsBegin() + nfds; ++iter) {
			handleEvent(server, *iter);
		}
		checkForTimeout(server);
	}
	LOG_INFO << "Received signal " << signalNumToName(g_signalStatus);
	if (g_signalStatus == SIGQUIT)
		shutdownServer(server);
}

/**
 * @brief Accepts new connections or handles existing ones

 * This function acts as a general dispatcher for handling events.
 * Checks if the event file descriptor is in the virtual servers map. If it is, it calls acceptConnections().
 * If the event mask contains EPOLLERR, it logs an error and closes the virtual server.
 *
 * If the event file descriptor is in the CGI connections map, it calls handleConnection() with the associated
 * referenced connection.
 *
 * Otherwise, it calls handleConnection() with the associated connection from the connections map.
 * If the event mask contains EPOLLERR or EPOLLHUP, the event mask is set to EPOLLIN.
 * This is done to handle errors and hangups that may occur during the processing of the connection.
 * The Server could then try to recv(), which will return 0 in case of EPOLLHUP or -1 in case of EPOLLERR,
 * resulting in a connection close.
 * @sa https://stackoverflow.com/a/29206631
 *
 * A connection should exist in the connections map, therefore it is not checked if the file descriptor
 * is in the connections map.
 *
 * @param event The epoll event to handle.
 */
void handleEvent(Server& server, struct epoll_event event)
{
	uint32_t eventMask = event.events;

	std::map<int, Socket>::const_iterator iter = server.getVirtualServers().find(event.data.fd);
	std::map<int, Connection*>::iterator cgiIter = server.getCGIConnections().find(event.data.fd);
	if (iter != server.getVirtualServers().end()) {
		if ((eventMask & EPOLLERR) != 0) {
			LOG_ERROR << "Error condition happened on the associated file descriptor of " << iter->second;
			server.removeVirtualServer(event.data.fd);
			return;
		}
		acceptConnections(server, iter->first, iter->second, eventMask);
	} else if (cgiIter != server.getCGIConnections().end()) {
		handleConnection(server, cgiIter->first, *cgiIter->second);
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

	const bool isWildcardServer = (serverSock.host == "0.0.0.0");

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

		const Socket clientSock = server.retrieveSocketInfo(addrCast);
		if (clientSock.host.empty() && clientSock.port.empty()) {
			close(clientFd);
			continue;
		}

		const Socket boundSock = isWildcardServer ? server.retrieveBoundSocketInfo(clientFd) : serverSock;
		if (boundSock.host.empty() && boundSock.port.empty()) {
			close(clientFd);
			continue;
		}

		if (!server.registerConnection(boundSock, clientFd, clientSock))
			continue;
	}
}

/**
 * @brief Handle a client connection.
 *
 * After epoll reports an event on a client socket, this function is called to handle the connection. The event from the
 * client can either be incoming (EPOLLIN) or outgoing (EPOLLOUT).
 * This function dispatches to the correct subfunction depending on the connection state:
 * 1. Connection::Idle (in): every new connection starts in this state after it connects. Also if the connection is
 * kept alive, it is in this state. When the client wants to send a request it changes the state to
 * Connection::ReceiveHeader.
 * 2. Connection::ReceiveHeader (in): when the connection wants to send a request. connectionReceiveHeader() reads the
 * request header from the client and parses it. If the request header is complete, it changes the state to
 * Connection::ReceiveBody, Connection::ReceiveFromCGI or Connection::BuildResponse.
 * 2. Connection::ReceiveBody (in): if the request header indicates a body, connectionReceiveBody() reads the body from
 * the client. If the body is complete, it changes the state to either Connection::SendToCGI or
 * Connection::BuildResponse.
 * 3. Connection::SendToCGI (out): no action is taken to send data to the CGI process in this function.
 * 4. Connection::ReceiveFromCGI (in): no action is taken to receive data from the CGI process in this function.
 * 5. Connection::BuildResponse (out): after a complete request (header and optional body) and optionally the body
 * generated by a CGI process is received connectionBuildResponse() builds the response for the client.
 * It then changes the state to Connection::SendResponse and also calls connectionSendResponse() to try to immediatly
 * send the response to the client.
 * 6. Connection::SendResponse (out): connectionSendResponse() sends the response to the client. This dispatch happens
 * only if the response is not completely sent the first time.
 * 7. Connection::Timeout (out): if the connection has timed out, connectionHandleTimeout() sets the request status to
 * timeout. Then it calls connectionBuildResponse() to build the error message, which then gets sent with
 * connectionSendResponse().
 * 7. Connection::Closed (not reached): the connection is closed and can be cleaned. This case should not be reached.
 *
 * @param server The server object to handle the connection for.
 * @param activeFd The file descriptor being handled.
 * @param connection The connection object to handle.
 */
void handleConnection(Server& server, const int activeFd, Connection& connection)
{
	LOG_DEBUG << "Handling connection: " << connection.m_clientSocket << " on fd: " << activeFd
			  << " for server: " << connection.m_serverSocket;

	switch (connection.m_status) {
	case (Connection::Idle):
		connection.m_status = Connection::ReceiveHeader;
		connectionReceiveHeader(server, activeFd, connection);
		break;
	case (Connection::ReceiveHeader):
		connectionReceiveHeader(server, activeFd, connection);
		break;
	case (Connection::ReceiveBody):
		connectionReceiveBody(server, activeFd, connection);
		break;
	case (Connection::SendToCGI):
		connectionSendToCGI(server, connection);
		break;
	case (Connection::ReceiveFromCGI):
		connectionReceiveFromCGI(server, connection);
		break;
	case (Connection::BuildResponse):
		connectionBuildResponse(server, activeFd, connection);
		break;
	case (Connection::SendResponse):
		connectionSendResponse(server, activeFd, connection);
		break;
	case (Connection::Timeout):
		connectionHandleTimeout(server, activeFd, connection);
		break;
	case (Connection::Closed):
		break;
	}
}

/**
 * @brief Receive request header from a client.
 *
 * This function reads data from the client socket using Server::readFromSocket().
 * The amount of bytes to read from the socket is determined by the bytes already received from the client. It can
 * never be bigger than the buffer size.
 * - If bytes read is -1, it indicates an internal server error
 * - If bytes read is 0, it indicates that the connection has been closed by the client
 * In both cases the clientFd is closed and the connection status is set to Closed.
 * If bytes read is greater than 0, the bytes received are added to the connection buffer and the bytes received of
 * the connection are updated as well as time since the last event is updated to the current time. If the buffer
 * contains a complete request header, the function handleCompleteRequestHeader() is called. If no complete request
 * was received and the received bytes match the buffer size, sets HTTP status code to 413 Request Header Fields Too
 * Large and status to BuildResponse, since the request header was too big.
 *
 * @param server The server object which handles the connection.
 * @param activeFd The file descriptor being handled.
 * @param connection The connection object to receive the request for.
 * @todo Implement body handling.
 * @todo make clientHeaderBufferSize configurable.
 */
void connectionReceiveHeader(Server& server, int activeFd, Connection& connection)
{
	if (activeFd != connection.m_clientFd)
		return;
	LOG_DEBUG << "Receive Request Header for: " << connection.m_clientSocket;

	std::vector<char>& buffer = server.getBuffer();
	if (buffer.capacity() < Server::s_clientHeaderBufferSize)
		buffer.resize(Server::s_clientHeaderBufferSize);

	const size_t bytesToRead = Server::s_clientHeaderBufferSize - connection.m_buffer.size();
	LOG_DEBUG << "Bytes to read: " << bytesToRead;

	const ssize_t bytesRead = server.readFromSocket(activeFd, &buffer[0], bytesToRead, 0);
	LOG_DEBUG << "Bytes read: " << bytesRead;

	if (bytesRead == -1) {
		LOG_ERROR << "Internal server error while reading from socket: " << connection.m_clientSocket;
		server.removeConnection(activeFd);
		return;
	}
	if (bytesRead == 0) {
		LOG_INFO << "Connection closed by client: " << connection.m_clientSocket;
		server.removeConnection(activeFd);
		return;
	}

	connection.m_buffer.append(buffer.begin(), buffer.begin() + bytesRead);
	connection.m_timeSinceLastEvent = std::time(0);
	if (isCompleteRequestHeader(connection.m_buffer)) {
		handleCompleteRequestHeader(server, activeFd, connection);
	} else {
		LOG_DEBUG << "Received partial request header: " << '\n' << connection.m_buffer;
		if (connection.m_buffer.size() >= Server::s_clientHeaderBufferSize) {
			LOG_ERROR << "Buffer full, didn't receive complete request header from " << connection.m_clientSocket;
			connection.m_request.httpStatus = StatusRequestHeaderFieldsTooLarge;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
		}
	}
}

/**
 * @brief Handles the complete request header received from a client.
 *
 * This function processes the complete request header received from a client,
 * parses the header, sets the active server configuration, and determines the
 * next steps based on the request type (e.g., CGI request, request with body).
 *
 * The function performs the following steps:
 * - Logs the received request header.
 * - Parses the request header.
 * - Sets the active server configuration based on the "Host" header.
 * - Finds the target resource for the request.
 * - Handles redirection if applicable by adding a "Location" header.
 * - Checks if the request method is allowed by the location.
 * - Handles CGI requests if applicable.
 * - Determines the next connection status based on the request type.
 *
 * If an error occurs during header parsing, the function logs the error, sets
 * the connection status to BuildResponse, and modifies the event to EPOLLOUT.
 *
 * If an error occurs while inserting the "Location" header, the function sets
 * the HTTP status code to 500 Internal Server Error, sets the connection status
 * to BuildResponse, and modifies the event to EPOLLOUT.
 *
 * If the request is for a location with a return directive, the function sets
 * the connection status to BuildResponse and modifies the event to EPOLLOUT.
 *
 * If the request is a CGI request, the function initializes and executes the
 * CGI handler, and registers the CGI file descriptors for EPOLLOUT and EPOLLIN
 * events.
 *
 * Depending on the request type, the function updates the connection status to
 * ReceiveBody, ReceiveFromCGI, or BuildResponse, and modifies the event to
 * EPOLLOUT if necessary.
 * @param server Reference to the Server instance handling the request.
 * @param clientFd File descriptor of the client connection.
 * @param connection Reference to the Connection instance representing the client connection.
 */
void handleCompleteRequestHeader(Server& server, int clientFd, Connection& connection)
{
	LOG_DEBUG << "Received complete request header: " << '\n' << connection.m_buffer;

	try {
		server.parseHeader(connection.m_buffer, connection.m_request);
	} catch (const RequestParser::HTTPErrorException& e) {
		LOG_ERROR << e.what();
		connection.m_request.httpStatus = e.statusCode;
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
		return;
	}

	std::map<std::string, std::string>::iterator iter = connection.m_request.headers.find("host");
	if (iter != connection.m_request.headers.end()) {
		if (!hasValidServerConfig(connection, server.getServerConfigs(), iter->second)) {
			LOG_ERROR << "Failed to set active server for " << connection.m_clientSocket;
			server.removeConnection(clientFd);
			return;
		}
	}
	LOG_DEBUG << "Active server: " << connection.m_serverSocket;

	server.findTargetResource(connection);

	if (connection.m_request.contentLength > connection.location->maxBodySize) {
		connection.m_request.httpStatus = StatusRequestEntityTooLarge;
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
		return;
	}

	if (connection.m_request.hasReturn) {
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
		return;
	}

	// Allow for non-existing files when POST is used
	if (connection.m_request.method == MethodPost && connection.m_request.httpStatus == StatusNotFound)
		connection.m_request.httpStatus = StatusOK;

	// Allow access to directories w/o autoindex when POST is used
	if (connection.m_request.method == MethodPost && connection.m_request.isDirectory)
		connection.m_request.httpStatus = StatusOK;

	// FIXME:
	// Allow directories to be deleted
	// if (connection.m_request.method == MethodDelete && connection.m_request.isDirectory)
	// 	connection.m_request.httpStatus = StatusOK;

	// bool array and method are scoped with enum Method
	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
	if (!connection.location->allowMethods[connection.m_request.method]) {
		connection.m_request.httpStatus = StatusMethodNotAllowed;
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
		return;
	}

	if (isCGIRequested(connection)) {
		connection.m_request.hasCGI = true;
		CGIHandler cgiHandler(connection, server.getProcessOps(), server.getFileSystemOps());
		if (connection.m_request.httpStatus == StatusInternalServerError) {
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(clientFd, EPOLLOUT);
			return;
		}
		cgiHandler.execute(server.getEpollFd(), server.getConnections(), server.getCGIConnections(), server.getVirtualServers());
	}

	LOG_DEBUG << "HTTP Status: " << connection.m_request.httpStatus;

	if (connection.m_request.httpStatus != StatusOK) {
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
		return;
	}

	if (connection.m_request.hasBody) {
		connection.m_status = Connection::ReceiveBody;
		connection.m_buffer.erase(0, connection.m_buffer.find("\r\n\r\n") + 4);
		if (!connection.m_buffer.empty())
			handleBody(server, clientFd, connection);
	} else if (connection.m_request.hasCGI) {
		connection.m_status = Connection::ReceiveFromCGI;
		if (!server.registerCGIFileDescriptor(connection.m_pipeFromCGIReadEnd, EPOLLIN, connection)) {
			connection.m_request.httpStatus = StatusInternalServerError;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(connection.m_clientFd, EPOLLOUT);
		} else
			server.removeEvent(clientFd);
	} else {
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(clientFd, EPOLLOUT);
	}
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

/**
 * @brief Checks if a CGI request is made based on the connection details.
 *
 * This function determines if a CGI request is being made by examining the
 * URI path of the request and comparing it with the CGI path and extension
 * specified in the connection's location.
 *
 * @param connection The connection object containing request and location details.
 * @return true if a CGI request is identified, false otherwise.
 */
bool isCGIRequested(Connection& connection)
{
	// FIXME: May need to iterate through multiple paths and extensions
	if (connection.location->cgiPath.empty() || connection.location->cgiExt.empty())
		return false;

	const size_t extPos = connection.m_request.uri.path.find(connection.location->cgiExt);

	// If extension is found and is at the end of the path or followed by a slash
	return (extPos != std::string::npos
		&& (extPos + connection.location->cgiExt.size() == connection.m_request.uri.path.size()
			|| connection.m_request.uri.path.at(extPos + connection.location->cgiExt.size()) == '/'));
}

/**
 * @brief Handles the reception of the request body from a client connection.
 *
 * This function reads data from the client socket and appends it to the connection's buffer.
 * It checks for various conditions such as errors during reading, client disconnection,
 * maximum body size limit, and completeness of the request body. Depending on these conditions,
 * it updates the connection status and modifies the event for the client socket.
 *
 * @param server Reference to the Server instance handling the connection.
 * @param activeFd The file descriptor being handled.
 * @param connection Reference to the Connection instance representing the client connection.
 *
 * @note If an error occurs while reading from the socket, the client connection is closed.
 * @note If the buffer size exceeds the maximum allowed client request body size, an error is logged
 * and the connection's HTTP status is set to StatusRequestEntityTooLarge.
 * @note If the request body is complete, it is parsed and the connection status is updated either to
 * Connection:SendToCGI or Connection::BuildResponse.
 */
void connectionReceiveBody(Server& server, int activeFd, Connection& connection)
{
	if (activeFd != connection.m_clientFd)
		return;
	LOG_DEBUG << "Receive Body for: " << connection.m_clientSocket;

	std::vector<char>& buffer = server.getBuffer();
	if (buffer.capacity() < Server::s_clientBodyBufferSize)
		buffer.resize(Server::s_clientBodyBufferSize);

	size_t bytesAvailable = 0;
	if (!connection.m_request.isChunked && connection.m_request.contentLength + 1 < connection.location->maxBodySize)
		bytesAvailable = connection.m_request.contentLength - connection.m_buffer.size();
	else
		bytesAvailable = connection.location->maxBodySize - connection.m_buffer.size();
	size_t bytesToRead = Server::s_clientBodyBufferSize;
	if (bytesAvailable <= Server::s_clientBodyBufferSize)
		bytesToRead -= bytesAvailable;
	LOG_DEBUG << "Bytes to read: " << bytesToRead;

	const ssize_t bytesRead = server.readFromSocket(activeFd, &buffer[0], bytesToRead, 0);
	LOG_DEBUG << "Bytes read: " << bytesRead;

	if (bytesRead == -1) {
		LOG_ERROR << "Internal server error while reading from socket: " << connection.m_clientSocket;
		server.removeConnection(activeFd);
		return;
	}
	if (bytesRead == 0) {
		LOG_INFO << "Connection closed by client: " << connection.m_clientSocket;
		server.removeConnection(activeFd);
		return;
	}

	connection.m_buffer.append(buffer.begin(), buffer.begin() + bytesRead);
	connection.m_timeSinceLastEvent = std::time(0);
	handleBody(server, activeFd, connection);
}

/**
 * @brief Handles the body of the HTTP request.
 *
 * This function processes the body of the HTTP request. If the complete body
 * is received, it parses the body and updates the connection status based on
 * whether CGI is involved. If only a partial body is received, it checks for
 * errors such as bad request or exceeding the maximum allowed body size.
 *
 * @param server Reference to the Server object.
 * @param activeFd The file descriptor of the active connection.
 * @param connection Reference to the Connection object.
 */
void handleBody(Server& server, int activeFd, Connection& connection)
{
	if (!connection.m_request.isChunked && connection.m_request.contentLength == connection.m_buffer.size())
		connection.m_request.isCompleteBody = true;

	if (connection.m_request.isChunked) {
		try {
			server.parseChunkedBody(connection.m_buffer, connection.m_request);
		} catch (const RequestParser::HTTPErrorException& e) {
			LOG_ERROR << e.what();
			connection.m_request.httpStatus = e.statusCode;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
			return;
		}
	}

	if (!connection.m_request.isCompleteBody) {
		LOG_DEBUG << "Received partial request body";
		// Printing body can be confusing for big files.
		// LOG_DEBUG << connection.m_buffer;
		if (connection.m_request.httpStatus != StatusOK) {
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
		} else if (connection.m_buffer.size() >= connection.location->maxBodySize) {
			LOG_ERROR << "Maximum allowed client request body size reached from " << connection.m_clientSocket;
			connection.m_request.httpStatus = StatusRequestEntityTooLarge;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
		} else if (!connection.m_request.isChunked && connection.m_request.contentLength < connection.m_buffer.size()) {
			LOG_ERROR << ERR_CONTENT_LENGTH;
			LOG_ERROR << "Content-Length: " << connection.m_request.contentLength
					  << ", Buffer size: " << connection.m_buffer.size();
			connection.m_request.httpStatus = StatusBadRequest;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
		}
		return;
	}

	LOG_DEBUG << "Received complete request body";
	// Printing body can be confusing for big files.
	// LOG_DEBUG << connection.m_buffer;
	if (!connection.m_request.isChunked)
		connection.m_request.body = connection.m_buffer;

	if (connection.m_request.hasMultipartFormdata) {
		try {
			server.decodeMultipartFormdata(connection.m_request);
		} catch (const RequestParser::HTTPErrorException& e) {
			LOG_ERROR << e.what();
			connection.m_request.httpStatus = e.statusCode;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
			return;
		}
	}

	if (connection.m_request.hasCGI) {
		connection.m_status = Connection::SendToCGI;
		if (connection.m_request.method == MethodPost
			&& !server.registerCGIFileDescriptor(connection.m_pipeToCGIWriteEnd, EPOLLOUT, connection)) {
			connection.m_request.httpStatus = StatusInternalServerError;
			connection.m_status = Connection::BuildResponse;
			server.modifyEvent(activeFd, EPOLLOUT);
			return;
		}
		server.removeEvent(activeFd);
	} else {
		connection.m_status = Connection::BuildResponse;
		server.modifyEvent(activeFd, EPOLLOUT);
	}
}
/**
 * @brief Sends the request body to the CGI process.
 *
 * This function handles sending the request body from the connection to the CGI process.
 * If the request body is empty, it logs an error, sets the HTTP status to internal server error,
 * updates the connection status to build response, modifies the event to EPOLLOUT, and removes
 * the CGI file descriptors from the server.
 *
 * If the request body is not empty, it attempts to write the body to the CGI process. If the write
 * operation fails, it logs an error, sets the HTTP status to internal server error, updates the
 * connection status to build response, modifies the event to EPOLLOUT, and removes the CGI file
 * descriptors from the server.
 *
 * If the entire body is successfully written to the CGI process, it logs a debug message, updates
 * the connection status to receive from CGI, clears the request body, and removes the write end
 * of the CGI pipe from the server.
 *
 * If only part of the body is written, it updates the request body to contain the remaining data.
 *
 * @param server Reference to the Server object.
 * @param connection Reference to the Connection object.
 */
void connectionSendToCGI(Server& server, Connection& connection)
{
	LOG_DEBUG << "Send to CGI for: " << connection.m_clientSocket;

	if (connection.m_request.body.empty()) {
		LOG_ERROR << "empty body: can't send to CGI";
		connection.m_request.httpStatus = StatusInternalServerError;
		connection.m_status = Connection::BuildResponse;
		server.addEvent(connection.m_clientFd, EPOLLOUT);
		server.removeCGIFileDescriptor(connection.m_pipeToCGIWriteEnd);
		return;
	}

	const ssize_t bytesSent = server.writeProcess(
		connection.m_pipeToCGIWriteEnd, connection.m_request.body.c_str(), connection.m_request.body.size());
	LOG_DEBUG << "Bytes sent to CGI: " << bytesSent;

	if (bytesSent == -1) {
		LOG_ERROR << "write(): can't send to CGI: " << std::strerror(errno);
		connection.m_request.httpStatus = StatusInternalServerError;
		connection.m_status = Connection::BuildResponse;
		server.addEvent(connection.m_clientFd, EPOLLOUT);
		server.removeCGIFileDescriptor(connection.m_pipeToCGIWriteEnd);
		return;
	}
	if (bytesSent == static_cast<ssize_t>(connection.m_request.body.size())) {
		LOG_DEBUG << "CGI: Full body sent";
		connection.m_status = Connection::ReceiveFromCGI;
		connection.m_request.body.clear();
		server.removeCGIFileDescriptor(connection.m_pipeToCGIWriteEnd);
		if (!server.registerCGIFileDescriptor(connection.m_pipeFromCGIReadEnd, EPOLLIN, connection)) {
			connection.m_request.httpStatus = StatusInternalServerError;
			connection.m_status = Connection::BuildResponse;
			server.addEvent(connection.m_clientFd, EPOLLOUT);
		}
		return;
	}
	connection.m_request.body.erase(0, bytesSent);
}

/**
 * @brief Handles receiving data from a CGI script and processes it accordingly.
 *
 * This function reads data from the CGI script's output pipe and appends it to the request body.
 * It also handles errors during the read operation and manages the connection state based on the
 * data received. If the read operation indicates that the CGI script has finished sending data,
 * the function will transition the connection to the response-building state and clean up the
 * associated file descriptors.
 *
 * @param server Reference to the Server instance managing the connection.
 * @param connection Reference to the Connection instance representing the client connection.
 */
void connectionReceiveFromCGI(Server& server, Connection& connection)
{
	LOG_DEBUG << "Receive from CGI for: " << connection.m_clientSocket;

	std::vector<char>& buffer = server.getBuffer();
	if (buffer.capacity() < Server::s_cgiBodyBufferSize)
		buffer.resize(Server::s_cgiBodyBufferSize);

	const ssize_t bytesRead
		= server.readProcess(connection.m_pipeFromCGIReadEnd, &buffer[0], Server::s_cgiBodyBufferSize);
	LOG_DEBUG << "Bytes read: " << bytesRead;

	if (bytesRead == -1) {
		LOG_ERROR << "read(): can't read from CGI: " << std::strerror(errno);
		connection.m_request.httpStatus = StatusInternalServerError;
		connection.m_status = Connection::BuildResponse;
		server.addEvent(connection.m_clientFd, EPOLLOUT);
		server.removeCGIFileDescriptor(connection.m_pipeFromCGIReadEnd);
		if (connection.m_pipeToCGIWriteEnd != -1)
			server.removeCGIFileDescriptor(connection.m_pipeToCGIWriteEnd);
		return;
	}
	if (bytesRead == 0) {
		LOG_DEBUG << "CGI: Full body received";
		connection.m_status = Connection::BuildResponse;
		server.addEvent(connection.m_clientFd, EPOLLOUT);
		server.removeCGIFileDescriptor(connection.m_pipeFromCGIReadEnd);
		if (connection.m_pipeToCGIWriteEnd != -1)
			server.removeCGIFileDescriptor(connection.m_pipeToCGIWriteEnd);
		int status = 0;
		if (server.waitForProcess(connection.m_cgiPid, &status, 0) == -1) {
			connection.m_request.httpStatus = StatusInternalServerError;
			return;
		}
		connection.m_cgiPid = -1;
		// Check if the child exited normally with exit() or returning from main()
		if (WIFEXITED(status)) { // NOLINT: misinterpretation by HIC++ standard
			int exitCode = WEXITSTATUS(status); // NOLINT: misinterpretation by HIC++ standard
			// Any child exit status unequal to 0 indicates unsuccessful completion of the process
			if (exitCode != 0) {
				LOG_ERROR << "child returned with: " << exitCode;
				connection.m_request.httpStatus = StatusInternalServerError;
				return;
			}
		} else if (WIFSIGNALED(status)) { // NOLINT: misinterpretation by HIC++ standard
			int signalNumber = WTERMSIG(status); // NOLINT: misinterpretation by HIC++ standard
			LOG_ERROR << "child terminated by signal: " << signalNumber << " (" << signalNumToName(signalNumber) << ")";
			connection.m_request.httpStatus = StatusInternalServerError;
			return;
		}
	}
	connection.m_request.body.append(buffer.begin(), buffer.begin() + bytesRead);
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
 * @param activeFd The file descriptor being handled.
 * @param connection The connection object to build the response for.
 */
void connectionBuildResponse(Server& server, int activeFd, Connection& connection)
{
	LOG_DEBUG << "BuildResponse for: " << connection.m_clientSocket;

	server.buildResponse(connection);
	connection.m_buffer.clear();
	connection.m_buffer = server.getResponse();
	connection.m_status = Connection::SendResponse;
	connectionSendResponse(server, activeFd, connection);
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
 * @param activeFd The file descriptor being handled.
 * @param connection The connection object to send the response for.
 */
void connectionSendResponse(Server& server, int activeFd, Connection& connection)
{
	LOG_DEBUG << "SendResponse for: " << connection.m_clientSocket << " on socket:" << activeFd;

	const ssize_t bytesToSend = static_cast<ssize_t>(connection.m_buffer.size());
	const ssize_t sentBytes = server.writeToSocket(activeFd, connection.m_buffer.c_str(), bytesToSend, 0);
	if (sentBytes == -1) {
		LOG_ERROR << "Internal server error";
		server.removeConnection(activeFd);
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
		server.removeConnection(activeFd);
	} else {
		LOG_DEBUG << "Connection alive";
		server.modifyEvent(activeFd, EPOLLIN);
		clearConnection(connection, server.getServerConfigs());
	}
}

/**
 * @brief Handles a timeout for a client connection.
 *
 * Handles a timeout for a client connection by setting the request status to StatusRequestTimeout.
 * Then it builds the error message by calling connectionBuildResponse().
 *
 * @param server The server object which handles the connection.
 * @param activeFd The file descriptor being handled.
 * @param connection The connection object to handle the timeout for.
 */
void connectionHandleTimeout(Server& server, int activeFd, Connection& connection)
{
	if (activeFd != connection.m_clientFd)
		return;

	LOG_DEBUG << "Timeout for: " << connection.m_clientSocket;

	connection.m_request.httpStatus = StatusRequestTimeout;

	connectionBuildResponse(server, activeFd, connection);
}

/**
 * @brief Iterates through all connections and checks if any have timed out.
 *
 * If the connection status is Closed, it is skipped.
 * The time since last event is saved in a variable to print it to the log and check
 * if it is greater than the timeout set for the server. If it is, the state is set to Timeout.
 * Additionally it is checked that the Connection is not currently
 * handling a CGI request. Only if it is not, the connection fd is
 * modified to EPOLLOUT. When the fd is ready to receive a message the timeout error
 * will be send.
 */
void checkForTimeout(Server& server)
{
	for (std::map<int, Connection>::iterator iter = server.getConnections().begin();
		 iter != server.getConnections().end(); ++iter) {
		const time_t timeSinceLastEvent = std::time(0) - iter->second.m_timeSinceLastEvent;
		LOG_DEBUG << iter->second.m_clientSocket << ": Time since last event: " << timeSinceLastEvent;
		if (timeSinceLastEvent > server.getClientTimeout()) {
			LOG_INFO << "Connection timeout: " << iter->second.m_clientSocket;
			LOG_DEBUG << "Timed out with status " << iter->second.m_status;
			if (iter->second.m_status == Connection::ReceiveFromCGI || iter->second.m_status == Connection::SendToCGI) {
				server.addEvent(iter->first, EPOLLOUT);
				if (iter->second.m_pipeToCGIWriteEnd != -1)
					server.removeCGIFileDescriptor(iter->second.m_pipeToCGIWriteEnd);
				if (iter->second.m_pipeFromCGIReadEnd != -1)
					server.removeCGIFileDescriptor(iter->second.m_pipeFromCGIReadEnd);
			} else
				server.modifyEvent(iter->first, EPOLLOUT);
			iter->second.m_status = Connection::Timeout;
		}
	}
}

/* ====== CLEANUP ====== */

/**
 * @brief Iterates through all connections, closes and removes idle ones.
 *
 * The for loop through the connections map has no increment statement because the
 * iterator is incremented in the loop body. If .erase() is called on an iterator,
 * it is invalidated.
 *
 * @param server The server object to cleanup idle connections for.
 */
void cleanupIdleConnections(Server& server)
{
	for (std::map<int, Connection>::iterator iter = server.getConnections().begin();
		 iter != server.getConnections().end();
		/* no iter*/) {
		if (iter->second.m_status == Connection::Idle) {
			close(iter->first);
			server.getConnections().erase(iter++);
		} else
			++iter;
	}
}

/**
 * @brief Performs a graceful shutdown of the server.
 *
 * Closes all virtual servers, cleans up idle and closed connections.
 * Then enters another event loop with two conditions:
 * 1. last signal received is SIGQUIT (which initiated graceful shutdown)
 * 2. as long as active connections exist.
 * This second loop always cleans up idle connections. It can also be interrupted with another signal, which aborts
 * the graceful shutdown.
 *
 * @param server The server object to shut down.
 * @sa https://pkg.go.dev/net/http#Server.Shutdown
 */
void shutdownServer(Server& server)
{
	LOG_DEBUG << "Closing all virtual servers";
	for (std::map<int, Socket>::iterator iter = server.getVirtualServers().begin();
		 iter != server.getVirtualServers().end(); ++iter) {
		server.removeEvent(iter->first);
		close(iter->first);
	}
	server.getVirtualServers().clear();

	LOG_DEBUG << "Cleanup idle connections";
	cleanupIdleConnections(server);

	LOG_DEBUG << "Waiting for connections to finish";
	while (g_signalStatus == SIGQUIT && !server.getConnections().empty()) {
		const int nfds = server.waitForEvents();

		for (std::vector<struct epoll_event>::const_iterator iter = server.eventsBegin();
			 iter != server.eventsBegin() + nfds; ++iter) {
			handleEvent(server, *iter);
		}
		checkForTimeout(server);
		cleanupIdleConnections(server);
	}
	if (g_signalStatus != SIGQUIT)
		LOG_INFO << "Graceful shutdown interrupted with signal " << g_signalStatus;
	else
		LOG_INFO << "Server shutdown gracefully";
}
