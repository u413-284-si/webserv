#pragma once

#include "Log.hpp"
#include "Socket.hpp"

#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @brief Policy class for socket functions.
 *
 * This class provides wrappers for functions regarding sockets. It provides functions to create a socket, bind a socket
 * to an address and port, listen on a socket and accept a connection on a socket.
 * It can also be mocked for testing purposes.
 */
class SocketPolicy {

public:
	SocketPolicy();
	virtual ~SocketPolicy();

	virtual struct addrinfo* resolveListeningAddresses(const std::string& host, const std::string& port) const;
	virtual int createListeningSocket(const struct addrinfo& addrinfo, int backlog) const;
	virtual Socket retrieveSocketInfo(struct sockaddr& sockaddr, socklen_t socklen) const;
	virtual int acceptSingleConnection(int sockfd, struct sockaddr* addr, socklen_t* addrlen) const;
	virtual ssize_t readFromSocket(int sockfd, char* buffer, size_t size, int flags) const;
	virtual ssize_t writeToSocket(int sockfd, const char* buffer, size_t size, int flags) const;

private:
	SocketPolicy(const SocketPolicy&);
	SocketPolicy& operator=(const SocketPolicy&);
};
