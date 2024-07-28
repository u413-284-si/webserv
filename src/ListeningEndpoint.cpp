#include "ListeningEndpoint.hpp"
#include "ConnectedEndpoint.hpp"
#include "Dispatcher.hpp"

ListeningEndpoint::ListeningEndpoint(const Socket& connection)
	: m_serverSock(connection)
{
}

ListeningEndpoint::~ListeningEndpoint() { close(m_serverSock.fd); }

void ListeningEndpoint::handleEvent(Dispatcher& dispatcher, uint32_t eventMask)
{
	LOG_DEBUG << "ListeningEndpoint " << m_serverSock.host << ':' << m_serverSock.port;

	if ((eventMask & EPOLLIN) == 0) {
		LOG_ERROR << "Received unknown event:" << eventMask;
		return;
	}
	struct sockaddr_storage clientAddr = { };
	socklen_t clientLen = sizeof(clientAddr);
	const int clientSock = accept(m_serverSock.fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // No more pending connections
		}
		LOG_ERROR << "accept: " << strerror(errno) << '\n';
		return;
	}

	char bufHost[NI_MAXHOST];
	char bufPort[NI_MAXSERV];
	const int ret = getnameinfo(reinterpret_cast<struct sockaddr*>(&clientAddr), clientLen, bufHost, NI_MAXHOST, bufPort, NI_MAXSERV,
		NI_NUMERICHOST | NI_NUMERICSERV);
	// we could also use standard values if getnameinfo() fails
	if (ret != 0) {
		LOG_ERROR << "getnameinfo: " << gai_strerror(ret) << '\n';
		close(clientSock);
		return;
	}
	const Socket connection = { clientSock, bufHost, bufPort };
	IEndpoint* endpoint = new ConnectedEndpoint(connection, m_serverSock);

	// Add client socket to epoll instance
	struct epoll_event event = { };
	event.events = EPOLLIN;
	event.data.ptr = static_cast<void*>(endpoint);

	if (!dispatcher.addEvent(clientSock, &event, endpoint)) {
		close(clientSock);
		delete endpoint;
	}
	LOG_INFO << "Connected to client: " << connection.host << ':' << connection.port;
}

time_t ListeningEndpoint::getTimeSinceLastEvent() const { return (0); }
