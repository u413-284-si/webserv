#include "Server.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.
 *
 * This constructor initializes a server instance by creating a server socket,
 * using TCP protocol SOCK_STREAM, and setting it to non-blocking mode.
 * It also creates an epoll instance for event handling.
 * If any of these operations fail, the constructor throws a std::runtime_error.
 * Then it bind() the server socket to a specific address and port, listen() on the server socket
 * Then the server socket is added to the epoll instance.
 *
 * @throws std::runtime_error if any of the socket creation, binding, listening,
 *         epoll creation, or epoll control operations fail.
 *
 */
Server::Server()
	: m_serverSock(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
	, m_epfd(epoll_create(1))
{
	if (m_serverSock < 0)
		throw std::runtime_error("socket");

	if (m_epfd < 0) {
		close(m_serverSock);
		throw std::runtime_error("epoll_create");
	}

	// Bind server socket
	struct sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET; // Communicates with IPv4
	serverAddr.sin_addr.s_addr = INADDR_ANY; // Accept any arriving ip addresses
	serverAddr.sin_port = htons(PORT); // Listen on specified port

	// NOLINTNEXTLINE: Ignore reinterpret_cast warning, as it is necessary for the bind function
	if (bind(m_serverSock, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
		close(m_serverSock);
		throw std::runtime_error("bind");
	}

	// Listen on server socket
	if (listen(m_serverSock, CONNECTION_QUEUE) < 0) {
		close(m_serverSock);
		throw std::runtime_error("listen");
	}

	// Add server socket to epoll instance
	struct epoll_event event = {};
	event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
	event.data.fd = m_serverSock;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_serverSock, &event) < 0) {
		close(m_serverSock);
		close(m_epfd);
		throw std::runtime_error("epoll_ctl: m_serverSock");
	}
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
Server::~Server()
{
	close(m_serverSock);
	close(m_epfd);
}

/**
 * @brief Copy constructor for the Server class.
 *
 * Private, should not be used.
 * @param ref Server instance to copy.
 */
Server::Server(const Server& ref)
	: m_serverSock(ref.m_serverSock)
	, m_epfd(ref.m_epfd)
{
}

/**
 * @brief Assignment operator for the Server class.
 *
 * Private, should not be used.
 * @param ref Server instance to copy.
 * @return Server& Reference to the current instance.
 */
Server& Server::operator=(const Server& ref)
{
	if (this == &ref)
		return *this;

	m_serverSock = ref.m_serverSock;
	m_epfd = ref.m_epfd;

	return *this;
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
	RequestParser parser;

	while (true) {
		struct epoll_event events[MAX_EVENTS];
		// Blocking call to epoll_wait
		int nfds = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
		if (nfds < 0)
			throw std::runtime_error("epoll_wait");

		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_serverSock)
				acceptConnection();
			else
				handleConnections(events[i].data.fd, parser);
		}
	}
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
void Server::acceptConnection() const
{
	while (true) {
		struct sockaddr_in clientAddr = {};
		socklen_t addrLen = sizeof(clientAddr);

		// NOLINTNEXTLINE: Ignore reinterpret_cast warning, as it is necessary for the accept function
		int clientSock = accept(m_serverSock, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);

		if (clientSock < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // No more pending connections
			std::cerr << "error: accept: " << strerror(errno) << "\n";
			break;
		}

		// Add client socket to epoll instance
		struct epoll_event event = {};
		event.events = EPOLLIN;
		event.data.fd = clientSock;
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, clientSock, &event) < 0) {
			std::cerr << "error: epoll_ctl: clientSock: " << strerror(errno) << "\n";
			close(clientSock);
		}
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
void Server::handleConnections(int clientSock, RequestParser& parser)
{
	LOG_DEBUG << "Handling connection on fd " << clientSock;
	// Handle client data
	char buffer[BUFFER_SIZE];
	HTTPRequest request;

	request.method = MethodCount;
	request.httpStatus = StatusOK;
	request.shallCloseConnection = false;

	Location location = {};
	location.path = "/";
	location.root = "/workspaces/webserv";
	location.index = "index.html";

	ServerConfig serverConfig;
	serverConfig.locations.push_back(location);

	ConfigFile configFile;
	configFile.serverConfigs.push_back(serverConfig);

	const ssize_t bytesRead = read(clientSock, buffer, BUFFER_SIZE);
	if (bytesRead < 0) {
		std::cerr << "error: read\n";
		close(clientSock);
	} else if (bytesRead == 0) {
		// Connection closed by client
		close(clientSock);
	} else {
		m_requestStrings[clientSock] += buffer;
		if (checkForCompleteRequest(clientSock)) {
			LOG_DEBUG << "Received complete request: " << '\n' << m_requestStrings[clientSock];
			try {
				parser.parseHttpRequest(m_requestStrings[clientSock], request);
				parser.clearParser();
			} catch (std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
			ResponseBuilder builder(configFile, m_fileSystemPolicy);
			builder.buildResponse(request);
			send(clientSock, builder.getResponse().c_str(), builder.getResponse().size(), 0 );
		}
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
