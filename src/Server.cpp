#include "Server.hpp"
#include "ResponseBuilder.hpp"

/* ====== HELPER FUNCTIONS ====== */

/**
 * @brief Set the socket to nonblocking mode
 *
 * Use fcntl to manipulate fd/sockets. Retrieve the currently set flags with
 * F_GETFL. Set the flag with with F_SETFL. Use bitwise OR to set the
 * O_NONBLOCK bit in "flags" while preserving all other bits.
 * @param sock	Socket to be manipulated
 */
static void setNonblocking(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		throw std::runtime_error("fcntl(F_GETFL)");
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl(F_SETFL)");
	}
}
/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructor for the Server class.
 *
 * This constructor initializes a server instance by creating a server socket,
 * setting it to non-blocking mode, binding it to a specific address and port,
 * listening on the server socket, creating an epoll instance for event handling,
 * and adding the server socket to the epoll instance.
 *
 * @throws std::runtime_error if any of the socket creation, binding, listening,
 *         epoll creation, or epoll control operations fail.
 *
 */
Server::Server()
{
	// Create server socket using TCP protocol SOCK_STREAM
	m_serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_serverSock < 0) {
		throw std::runtime_error("socket");
	}

	// Set server socket to non-blocking
	setNonblocking(m_serverSock);

	// Bind server socket
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET; // Communicates with IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // Accept any arriving ip addresses
	server_addr.sin_port = htons(PORT); // Listen on specified port
	if (bind(m_serverSock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		close(m_serverSock);
		throw std::runtime_error("bind");
	}

	// Listen on server socket
	if (listen(m_serverSock, 10) < 0) {
		close(m_serverSock);
		throw std::runtime_error("listen");
	}

	// Create epoll instance
	m_epfd = epoll_create1(0);
	if (m_epfd < 0) {
		close(m_serverSock);
		throw std::runtime_error("epoll_create1");
	}

	// Add server socket to epoll instance
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
	ev.data.fd = m_serverSock;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_serverSock, &ev) < 0) {
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
	while (1) {
		struct epoll_event events[MAX_EVENTS];
		// Blocking call to epoll_wait
		int nfds = epoll_wait(m_epfd, events, MAX_EVENTS, -1);
		if (nfds < 0)
			throw std::runtime_error("epoll_wait");

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == m_serverSock)
				acceptConnection();
			else
				handleConnections(events[n].data.fd);
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
void Server::acceptConnection()
{
	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t addr_len = sizeof(clientAddr);
		int clientSock = accept(m_serverSock, (struct sockaddr*)&clientAddr, &addr_len);
		if (clientSock < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break; // No more pending connections
			} else {
				std::cerr << "error: accept: " << strerror(errno) << "\n";
				break;
			}
		}

		// Add client socket to epoll instance
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = clientSock;
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, clientSock, &ev) < 0) {
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
void Server::handleConnections(int clientSock)
{
	// Handle client data
	char buffer[BUFFER_SIZE];
	int bytesRead = read(clientSock, buffer, BUFFER_SIZE);
	if (bytesRead < 0) {
		std::cerr << "error: read\n";
		close(clientSock);
	} else if (bytesRead == 0) {
		// Connection closed by client
		close(clientSock);
	} else {
		Location location;
		location.path = "/";
		location.root = "/workspaces/webserv";
		location.isAutoindex = true;
		ServerConfig serverConfig;
		serverConfig.locations.push_back(location);
		ConfigFile configFile;
		configFile.serverConfigs.push_back(serverConfig);
		HTTPRequest request;
		request.body = buffer;
		request.uri.path = "/";
		ResponseBuilder responseBuilder(configFile, FileSystemPolicy());
		responseBuilder.buildResponse(request);
		write(clientSock, responseBuilder.getResponse().c_str(), responseBuilder.getResponse().size());
		// FIXME: check requestString for complete HTTP request.
		// If yes, hand over to request parser and then clear the requestString.
		// If no, concatenate to requestString and exit, only to come back for remainder.
		// if (checkRequestString()) {
		// 	RequestParser	parseSoGood;

		// 	parseSoGood.parse();
		// 	m_requestString[clientSock].clear();
		// } else
		// 		m_requestStrings[clientSock] += buffer;
		//
	}
}
