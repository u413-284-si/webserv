#include "ServerEndpoint.hpp"
#include "ClientEndpoint.hpp"
#include "Dispatcher.hpp"

ServerEndpoint::ServerEndpoint(const Connection& connection)
	: m_connection(connection)
{
}

ServerEndpoint::~ServerEndpoint() { close(m_connection.fd); }

void ServerEndpoint::handleEvent(Dispatcher& dispatcher)
{
	LOG_DEBUG << "Endpoint " << m_connection.host << ':' << m_connection.port;

	struct sockaddr_storage clientAddr = { };
	socklen_t clientLen = sizeof(clientAddr);
	const int clientSock = accept(m_connection.fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // No more pending connections
		}
		LOG_ERROR << "accept: " << strerror(errno) << '\n';
		return;
	}

	char bufHost[NI_MAXHOST];
	char bufPort[NI_MAXSERV];
	getnameinfo(reinterpret_cast<struct sockaddr*>(&clientAddr), clientLen, bufHost, NI_MAXHOST, bufPort, NI_MAXSERV,
		NI_NUMERICHOST | NI_NUMERICSERV);
	const Connection connection = { clientSock, bufHost, bufPort };
	IEndpoint* endpoint = new ClientEndpoint(connection, m_connection);

	// Add client socket to epoll instance
	struct epoll_event event = { };
	event.events = EPOLLIN | EPOLLOUT;
	event.data.ptr = static_cast<void*>(endpoint);

	if (!dispatcher.addEvent(clientSock, &event, endpoint)) {
		close(clientSock);
		delete endpoint;
	}
	LOG_INFO << "Added client endpoint: " << connection.host << ':' << connection.port;
}

time_t ServerEndpoint::getTimeSinceLastEvent() const { return (0); }

void ServerEndpoint::setTimeSinceLastEvent() { m_TimeSinceLastEvent = std::time(0); }
