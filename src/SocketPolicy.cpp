#include "SocketPolicy.hpp"

SocketPolicy::SocketPolicy() { }

SocketPolicy::~SocketPolicy() { }

/**
 * @brief Resolves the passed host and port to a list of addresses suitable for listening.
 *
 * Uses getaddrinfo() to get a list of address information for the provided host and port. Since a host/port combination
 * can resolve to multiple addresses, getaddrinfo() returns a linked list of addrinfo structures. If the passed host is
 * empty or "*", it sets the passed node to NULL, which will return an address suitable for accepting any network
 * connections.
 *
 * An addrinfo hints struct is used to specify the criteria for selecting the address with the following values:
 * - .ai_flags - AI_PASSIVE: The returned address will be suitable to bind(2) a socket which can accept(2) connections.
 *   With this flag set: If node is NULL, returned address (INADDR_ANY) will be suitable to accept any network
 *   connections. W/o the flag the returned address is a loopback not allowing external connections.
 * - .ai_family - AF_UNSPEC: Allow IPv4 or IPv6.
 * - .ai_socktype - SOCK_STREAM: TCP uses SOCK_STREAM.
 * - .ai_protocol - 0: Accept any protocol.
 * - .ai_addrlen - sizeof(struct addrinfo): Not used.
 * - .ai_addr - NULL: Not used.
 * - .ai_canonname - NULL: Not used.
 * - .ai_next - NULL: Not used.
 *
 * If getaddrinfo() returns an error, it logs the error message and returns NULL.
 *
 * @sa man getaddrinfo
 * @param host The host address to resolve.
 * @param port The port number to resolve.
 * @return struct addrinfo* A pointer to the list of addresses if successful, NULL otherwise.
 */
struct addrinfo* SocketPolicy::resolveListeningAddresses(const std::string& host, const std::string& port) const
{
	const char* node = NULL;

	if (!host.empty() || host != "*")
		node = host.c_str();

	struct addrinfo hints = {
		AI_PASSIVE, /* .ai_flags */
		AF_UNSPEC, /*.ai_family  */
		SOCK_STREAM, /* .ai_socktype */
		0, /* .ai_protocol */
		sizeof(struct addrinfo), /* .ai_addrlen */
		NULL, /* .ai_addr */
		NULL, /* .ai_canonname */
		NULL /* .ai_next */
	};

	struct addrinfo* list = NULL;
	const int result = getaddrinfo(node, port.c_str(), &hints, &list);
	if (result != 0) {
		LOG_ERROR << "getaddrinfo(): " << gai_strerror(result);
		return NULL;
	}

	return list;
}

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
int SocketPolicy::createListeningSocket(const struct addrinfo& addrinfo, int backlog) const
{
	unsigned int socktype = addrinfo.ai_socktype;
	socktype |= SOCK_NONBLOCK;

	const int newFd = socket(addrinfo.ai_family, static_cast<int>(socktype), addrinfo.ai_protocol);
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

	if (-1 == bind(newFd, addrinfo.ai_addr, addrinfo.ai_addrlen)) {
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
Socket SocketPolicy::retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const
{
	char bufferHost[NI_MAXHOST];
	char bufferPort[NI_MAXSERV];

	const int ret = getnameinfo(
		&sockaddr, socklen, bufferHost, NI_MAXHOST, bufferPort, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret != 0) {
		LOG_ERROR << "getnameinfo(): " << gai_strerror(ret);
		const Socket errSock = { "", "" };
		return (errSock);
	}

	const Socket newSock = { bufferHost, bufferPort };

	return (newSock);
}

/**
 * @brief Accept a connection on a socket.
 *
 * Accepts a connection on the provided socket file descriptor and retrieves the address information of the client.
 * The provided socket must be in non-blocking mode.
 *
 * @param sockfd The file descriptor of the socket.
 * @param addr The address information of the client.
 * @param addrlen The length of the address information.
 *
 * @retval The file descriptor of the new socket if successful.
 * @retval -1 If accept() failed and errno is not EAGAIN or EWOULDBLOCK.
 * @return -2 No more pending connections.
 */
int SocketPolicy::accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const
{
	const int newFd = accept(sockfd, addr, addrlen);
	if (newFd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (-2); // No more pending connections
		LOG_ERROR << "accept(): " << strerror(errno);
	}

	return newFd;
}

/**
 * @brief Read from a socket.
 *
 * Reads data from the provided socket file descriptor into the provided buffer.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer.
 *
 * @retval The number of bytes read if successful.
 * @retval -1 If read() failed.
 */
ssize_t SocketPolicy::readFromSocket(int sockfd, char* buffer, size_t size, int flags) const
{
	const ssize_t bytesRead = recv(sockfd, buffer, size, flags);
	if (bytesRead == -1) {
		LOG_ERROR << "recv(): " << strerror(errno);
	}

	return bytesRead;
}

/**
 * @brief Write to a socket.
 *
 * Writes data from the provided buffer to the provided socket file descriptor.
 *
 * @param sockfd The file descriptor of the socket.
 * @param buffer The buffer to write the data from.
 * @param size The size of the buffer.
 *
 * @retval The number of bytes written if successful.
 * @retval -1 If write() failed.
 */
ssize_t SocketPolicy::writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const
{
	const ssize_t bytesWritten = send(sockfd, buffer, size, flags);
	if (bytesWritten == -1) {
		LOG_ERROR << "send(): " << strerror(errno);
	}

	return bytesWritten;
}
