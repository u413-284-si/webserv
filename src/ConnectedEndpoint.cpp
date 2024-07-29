#include "ConnectedEndpoint.hpp"
#include "Dispatcher.hpp"

ConnectedEndpoint::ConnectedEndpoint(const Connection& connection)
	: m_clientSock(connection.clientSock)
	, m_serverSock(connection.serverSock)
	, m_TimeSinceLastEvent(std::time(0))
{
}

ConnectedEndpoint::~ConnectedEndpoint() { close(m_clientSock.fd); }

void ConnectedEndpoint::handleEvent(Dispatcher& dispatcher, uint32_t eventMask)
{
	LOG_DEBUG << "ConnectedEndpoint with client: " << m_clientSock.host << ':' << m_clientSock.port;

	if ((eventMask & EPOLLIN) != 0) {
		LOG_DEBUG << "Received read event";
		char buffer[1024];
		const ssize_t bytesRead = recv(m_clientSock.fd, buffer, 1024, 0);
		if (bytesRead < 0) {
			LOG_ERROR << "recv: " << strerror(errno);
			return (dispatcher.removeEvent(m_clientSock.fd, this));
		}
		if (bytesRead == 0) {
			LOG_INFO << "Connection closed by client: " << m_clientSock.host << ':' << m_clientSock.port;
			return (dispatcher.removeEvent(m_clientSock.fd, this));
		}
		m_buffer += std::string(buffer, bytesRead);
		if (m_buffer.find("\r\n\r\n") != std::string::npos) {
			LOG_DEBUG << "Received full request from client: " << m_clientSock.host << ':' << m_clientSock.port;
			struct epoll_event event = { };
			event.events = EPOLLOUT;
			event.data.ptr = static_cast<void*>(this);
			dispatcher.modifyEvent(m_clientSock.fd, &event);
		}
		// Wait for more data
	}
	else if ((eventMask & EPOLLOUT) != 0) {
		LOG_DEBUG << "Received write event";
		std::stringstream response;
		response << "HTTP/1.1 200 OK\r\nContent-Length: " << m_buffer.size() << "\r\n\r\n";
		response << m_buffer;
		const ssize_t bytesSent = send(m_clientSock.fd, response.str().c_str(), response.str().size(), 0);
		if (bytesSent < 0) {
			LOG_ERROR << "send: " << strerror(errno) << '\n';
			return (dispatcher.removeEvent(m_clientSock.fd, this));
		}
		LOG_INFO << "Sent response to client: " << m_clientSock.host << ':' << m_clientSock.port;
		// we need to check if connection should be closed.
		// If so, we should remove it from epoll
		LOG_INFO << "Closing connection to client: " << m_clientSock.host << ':' << m_clientSock.port;
		return (dispatcher.removeEvent(m_clientSock.fd, this));
		/* else we would modify to read again
		struct epoll_event event = { };
		event.events = EPOLLIN;
		event.data.ptr = static_cast<void*>(this);
		dispatcher.modifyEvent(m_connection.fd, &event);
		*/
	}
	else { // we don't know this event
		LOG_ERROR << "Received unknown event:" << eventMask;
	}
	setTimeSinceLastEvent();
}

time_t ConnectedEndpoint::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

void ConnectedEndpoint::setTimeSinceLastEvent() { m_TimeSinceLastEvent = std::time(0); }
