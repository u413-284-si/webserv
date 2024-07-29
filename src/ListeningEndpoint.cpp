#include "ListeningEndpoint.hpp"
#include "ConnectedEndpoint.hpp"
#include "Dispatcher.hpp"

ListeningEndpoint::ListeningEndpoint(const Socket& serverSock)
	: m_serverSock(serverSock)
{
	m_host.resize(NI_MAXHOST);
	m_port.resize(NI_MAXSERV);
}

ListeningEndpoint::~ListeningEndpoint() { close(m_serverSock.fd); }

ListeningEndpoint::ListeningEndpoint(const ListeningEndpoint& ref)
	: m_serverSock(ref.m_serverSock)
{
}

ListeningEndpoint& ListeningEndpoint::operator=(const ListeningEndpoint& ref)
{
	if (this != &ref) {
		m_serverSock = ref.m_serverSock;
	}
	return (*this);
}

void ListeningEndpoint::handleEvent(Dispatcher& dispatcher, uint32_t eventMask)
{
	LOG_DEBUG << "ListeningEndpoint " << m_serverSock;

	if ((eventMask & EPOLLIN) == 0) {
		LOG_ERROR << "Received unknown event:" << eventMask;
		return;
	}
	struct sockaddr_storage clientAddr = {};
	socklen_t clientLen = sizeof(clientAddr);
	// NOLINTNEXTLINE: we need to use reinterpret_cast to convert sockaddr_storage to sockaddr
	const int clientSock = accept(m_serverSock.fd, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
	if (clientSock < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // No more pending connections
		}
		LOG_ERROR << "accept(): " << strerror(errno);
		return;
	}

	// NOLINTNEXTLINE: we need to use reinterpret_cast to convert sockaddr_storage to sockaddr
	const int ret = getnameinfo(reinterpret_cast<struct sockaddr*>(&clientAddr), clientLen, &m_host[0], NI_MAXHOST,
		&m_port[0], NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	// we could also use standard values if getnameinfo() fails
	if (ret != 0) {
		LOG_ERROR << "getnameinfo(): " << gai_strerror(ret);
		close(clientSock);
		return;
	}

	const Connection connection = { { clientSock, m_host, m_port }, m_serverSock };
	IEndpoint* endpoint = new ConnectedEndpoint(connection);

	// Add client socket to epoll instance
	struct epoll_event event = {};
	event.events = EPOLLIN;
	event.data.ptr = static_cast<void*>(endpoint);

	if (!dispatcher.addEvent(clientSock, &event, endpoint)) {
		close(clientSock);
		delete endpoint;
	}
	LOG_INFO << "Created connected endpoint: " << connection.clientSock << " for server: " << connection.serverSock;
}

time_t ListeningEndpoint::getTimeSinceLastEvent() const { return (0); }
