#pragma once

#include "Log.hpp"
#include "Socket.hpp"
#include "utilities.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @brief Wrapper class for process-related functions.
 *
 * This class provides wrappers for functions interacting with sockets. The following functions are wrapped:
 * - getaddrinfo()
 * - socket()
 * - setsockopt()
 * - bind()
 * - listen()
 * - ntohs()
 * - getsockname()
 * - accept()
 * - recv()
 * - send()
 * It can also be mocked for testing purposes.
 */
class SocketOps {

public:
	SocketOps();
	virtual ~SocketOps();

	virtual struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	virtual int createListeningSocket(const struct addrinfo* addrinfo, int backlog) const;
	virtual Socket retrieveSocketInfo(struct sockaddr* sockaddr) const;
	virtual Socket retrieveBoundSocketInfo(int sockfd) const;
	virtual int acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;
	virtual ssize_t readFromSocket(int sockfd, char* buffer, size_t size, int flags) const;
	virtual ssize_t writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const;

private:
	SocketOps(const SocketOps&);
	SocketOps& operator=(const SocketOps&);
};
