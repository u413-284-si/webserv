#include "Connection.hpp"

Connection::Connection(const Socket& server, const Socket& client)
	: m_server(server)
	, m_client(client)
	, m_TimeSinceLastEvent(std::time(0))
	, m_isActive(true)
{
}

time_t Connection::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

bool Connection::isActive() const { return m_isActive; }

Socket Connection::getClient() const { return m_client; }

Socket Connection::getServer() const { return m_server; }
