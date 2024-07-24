#include "ClientEndpoint.hpp"

ClientEndpoint::ClientEndpoint(const Connection& connection, const Connection& server)
	: m_connection(connection)
	, m_server(server)
{
}

ClientEndpoint::~ClientEndpoint() { close(m_connection.fd); }

void ClientEndpoint::handleEvent(Dispatcher& dispatcher)
{

}

time_t ClientEndpoint::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

void ClientEndpoint::setTimeSinceLastEvent() { m_TimeSinceLastEvent = std::time(0); }