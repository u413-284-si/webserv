#include "ServerEndpoint.hpp"

ServerEndpoint::ServerEndpoint(const Connection& connection)
	: m_connection(connection)
{
}

ServerEndpoint::~ServerEndpoint()
{
	close (m_connection.fd);
}

void ServerEndpoint::handleEvent(Dispatcher& dispatcher)
{
	struct sockaddr_in	clientAddr;
	socklen_t			addr_len = sizeof(clientAddr);
	int	clientSock = accept(
		m_connection.fd, (struct sockaddr*)&clientAddr, &addr_len);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // No more pending connections
		} else {
			LOG_ERROR << "error: accept: " << strerror(errno) << '\n';
			return;
		}
	}

	// Add client socket to epoll instance
	struct epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.fd = clientSock;
	if (!dispatcher.addEvent(clientSock, &ev)) {
		close(clientSock);
	}
}
