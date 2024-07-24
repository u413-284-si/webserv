#include "ClientEndpoint.hpp"
#include "Dispatcher.hpp"

ClientEndpoint::ClientEndpoint(const Connection& connection, const Connection& server)
	: m_connection(connection)
	, m_server(server)
	, m_TimeSinceLastEvent(std::time(0))
{
}

ClientEndpoint::~ClientEndpoint() { close(m_connection.fd); }

void ClientEndpoint::handleEvent(Dispatcher& dispatcher, uint32_t eventMask)
{
	LOG_DEBUG << "ClientEndpoint " << m_connection.host << ':' << m_connection.port;

	if ((eventMask & EPOLLIN) != 0) {
		LOG_DEBUG << "Received read event";
		char buffer[1024];
		const ssize_t bytesRead = recv(m_connection.fd, buffer, 1024, 0);
		if (bytesRead < 0) {
			LOG_ERROR << "recv: " << strerror(errno) << '\n';
			dispatcher.removeEvent(m_connection.fd, this);
		} else if (bytesRead == 0) {
			LOG_INFO << "Connection closed by client: " << m_connection.host << ':' << m_connection.port;
			dispatcher.removeEvent(m_connection.fd, this);
		}
		m_buffer += std::string(buffer, bytesRead);
		if (m_buffer.find("\r\n\r\n") != std::string::npos) {
			LOG_DEBUG << "Received full request from client: " << m_connection.host << ':' << m_connection.port;
			struct epoll_event event = { };
			event.events = EPOLLOUT;
			event.data.ptr = static_cast<void*>(this);
			dispatcher.modifyEvent(m_connection.fd, &event);
		}
		// Wait for more data
	}
	else if ((eventMask & EPOLLOUT) != 0) {
		LOG_DEBUG << "Received write event";
		std::stringstream response;
		response << "HTTP/1.1 200 OK\r\nContent-Length: " << m_buffer.size() << "\r\n\r\n";
		response << m_buffer;
		const ssize_t bytesSent = send(m_connection.fd, response.str().c_str(), response.str().size(), 0);
		if (bytesSent < 0) {
			LOG_ERROR << "send: " << strerror(errno) << '\n';
			dispatcher.removeEvent(m_connection.fd, this);
		}
		else {
			LOG_INFO << "Sent response to client: " << m_connection.host << ':' << m_connection.port;
			dispatcher.removeEvent(m_connection.fd, this);
		}
	}
	else {
		LOG_ERROR << "Received unknown event:" << eventMask;
	}
	setTimeSinceLastEvent();
}

time_t ClientEndpoint::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

void ClientEndpoint::setTimeSinceLastEvent() { m_TimeSinceLastEvent = std::time(0); }
