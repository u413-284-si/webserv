#include "Server.hpp"
#include "ConnectedEndpoint.hpp"
#include "Connection.hpp"
#include <cmath>

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.
 *
 * This constructor initializes an epoll instance for event handling.
 * Then it calls Server::init() to initialize virtual servers, as they are
 * defined in the configuration file.
 * If any of these operations fail, the constructor throws a std::runtime_error.
 *
 * @throws std::runtime_error if epoll_create or Server::init() fail.
 *
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
	if (!init())
		throw std::runtime_error("Failed to initialize virtual servers");
	m_buffer.resize(s_bufferSize);
}

/**
 * @brief Destructor for the Server class.
 *
 * This destructor cleans up resources associated with the server instance by
 * closing the server socket and the epoll instance.
 *
 * @details The destructor performs the following cleanup actions:
 *
 * 1. Closes the server socket to release the bound port and stop accepting
 * new connections.
 * 2. Closes the epoll instance to release associated resources and stop
 * monitoring events.
 */
Server::~Server() { close(m_epfd); }

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
			handleEvent(iter->data.fd, iter->events);
		}
		handleTimeout();
	}
}

void Server::handleTimeout()
{
	for (std::map<int, Connection>::iterator iter = m_connections.begin(); iter != m_connections.end();
		/* no increment */) {
		time_t timeSinceLastEvent = iter->second.getTimeSinceLastEvent();
		LOG_DEBUG << iter->second.getClient() << ": Time since last event: " << timeSinceLastEvent;
		if (timeSinceLastEvent > m_clientTimeout) {
			LOG_INFO << "Connection timeout: " << iter->second.getClient();
			removeEvent(m_epfd, iter->first);
			iter->second.closeConnection();
			m_connections.erase(iter++);
		} else
			++iter;
	}
}

bool Server::init()
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

	if (m_virtualServers.empty()) {
		LOG_ERROR << "No virtual servers added";
		return false;
	}

	LOG_INFO << "Finished setting up virtual servers";
	return true;
}

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

void Server::handleEvent(int eventfd, uint32_t eventMask)
{
	if ((eventMask & EPOLLERR) != 0) {
		LOG_DEBUG << "epoll_wait: EPOLLERR";
		eventMask = EPOLLOUT;
	} else if ((eventMask & EPOLLHUP) != 0) {
		LOG_DEBUG << "epoll_wait: EPOLLHUP";
		eventMask = EPOLLOUT;
	}
	std::map<int, Socket>::const_iterator iter = m_virtualServers.find(eventfd);
	if (iter != m_virtualServers.end())
		acceptConnections(iter->second, eventMask);
	else
		handleConnections(m_connections.at(eventfd));
}

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
 * @brief Accept new connections
 *
 * A new connection is accepted, its address and port saved in _cliendAddr and a
 * file descriptor for the new client socket is saved in clientSock. This
 * information is updated each time a new connection is accepted. The
 * clientSock is added to the list of fds watched by epoll.
 *
 * The while-loop allows for multiple connections to be accepted in one call
 * of this function. It is broken when accept sets errno to EAGAIN or
 * EWOULDBLOCK (equivalent error codes indicating that a non-blocking operation
 * would normally block), meaning that no more connections are pending.
 */
void Server::acceptConnections(const Socket& serverSock, uint32_t eventMask)
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

	m_connections.insert(std::make_pair(clientSock.fd, Connection(serverSock, clientSock)));

	LOG_INFO << "New Connection: " << clientSock << " for server: " << serverSock;
	return true;
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
void Server::handleConnections(Connection& connection)
{
	LOG_DEBUG << "Handling connection: " << connection.getClient() << " for server: " << connection.getServer();

	switch (connection.getStatus()) {
	case Connection::ReceiveRequest:
		receiveRequest(connection);
		break;
	case Connection::ReceiveBody:
		receiveBody(connection);
		break;
	case Connection::BuildResponse:
		buildResponse(connection);
		break;
	case Connection::SendResponse:
		sendResponse(connection);
		break;
	}
}

bool Server::checkForCompleteRequest(int clientSock)
{
	const size_t headerEndPos = m_requestStrings[clientSock].find("\r\n\r\n");

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

int waitForEvents(int epfd, std::vector<struct epoll_event>& events, int timeout)
{
	LOG_DEBUG << "Waiting for events";

	const int nfds = epoll_wait(epfd, &events[0], static_cast<int>(events.size()), timeout);
	if (nfds == -1)
		throw std::runtime_error("epoll_wait:" + std::string(strerror(errno)));
	if (nfds == 0)
		LOG_DEBUG << "epoll_wait: Timeout";
	else
		LOG_DEBUG << "epoll_wait: " << nfds << " events";
	return nfds;
}

bool addEvent(int epfd, int newfd, epoll_event* event)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_ADD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Added new fd: " << newfd;

	return true;
}

void removeEvent(int epfd, int delfd)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, delfd, NULL)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_DEL: " << strerror(errno);
		return;
	}
	LOG_DEBUG << "epoll_ctl: Removed fd: " << delfd;
}

bool modifyEvent(int epfd, int modfd, epoll_event* event)
{
	if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, modfd, event)) {
		LOG_ERROR << "epoll_ctl: EPOLL_CTL_MOD: " << strerror(errno);
		return false;
	}
	LOG_DEBUG << "epoll_ctl: Modified fd: " << modfd;
	return true;
}

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

void Server::receiveRequest(Connection& connection)
{
	LOG_DEBUG << "Receiving request from: " << connection.getClient();

	const ssize_t bytesRead = recv(connection.getClient().fd, &m_buffer[0], static_cast<int>(m_buffer.size()), 0);

	if (bytesRead == -1) {
		LOG_ERROR << "recv()" << strerror(errno);
		connection.closeConnection();
		return;
	}

	if (bytesRead == 0) {
		LOG_INFO << "Connection closed by client: " << connection.getClient();
		connection.closeConnection();
		return;
	}

	m_requestStrings[connection.getClient().fd] += m_buffer.substr(0, bytesRead);
	connection.updateBytesReceived(bytesRead);

	if (!checkForCompleteRequest(connection.getClient().fd)) {
		LOG_DEBUG << "Received partial request: " << '\n' << m_requestStrings[connection.getClient().fd];
		if (connection.getBytesReceived() > m_buffer.size()) {
			LOG_ERROR << "Request too large: " << connection.getClient();
			connection.closeConnection();
		}
		return;
	}

	LOG_DEBUG << "Received complete request: " << '\n' << m_requestStrings[connection.getClient().fd];

	HTTPRequest request;

	request.method = MethodCount;
	request.httpStatus = StatusOK;
	request.shallCloseConnection = false;
	try {
		m_requestParser.parseHttpRequest(m_requestStrings[connection.getClient().fd], request);
		m_requestParser.clearParser();
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
	}
	connection.setStatus(Connection::BuildResponse);
	struct epoll_event event = {};
	event.events = EPOLLOUT;
	event.data.fd = connection.getClient().fd;
	modifyEvent(m_epfd, connection.getClient().fd, &event);
}

void Server::receiveBody(Connection& connection)
{

}

void Server::buildResponse(Connection& connection)
{
	m_responseBuilder.buildResponse(request);
	send(connection.getClient().fd, m_responseBuilder.getResponse().c_str(),
		m_responseBuilder.getResponse().size(), 0);
}

void Server::sendResponse(Connection& connection)
{

}