#include "Connection.hpp"

Connection::Connection(const Socket& server, const Socket& client)
	: m_server(server)
	, m_client(client)
	, m_TimeSinceLastEvent(std::time(0))
	, m_status(ReceiveRequest)
	, m_bytesReceived(0)
{
}

void Connection::closeConnection() const { close(m_client.fd); }

void Connection::setStatus(ConnectionStatus status) { m_status = status; }

void Connection::updateBytesReceived(std::size_t bytesReceived) { m_bytesReceived += bytesReceived; }

Socket Connection::getServer() const { return m_server; }

Socket Connection::getClient() const { return m_client; }

time_t Connection::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

Connection::ConnectionStatus Connection::getStatus() const { return m_status; }

std::size_t Connection::getBytesReceived() const { return m_bytesReceived; }
