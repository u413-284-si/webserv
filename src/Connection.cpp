#include "Connection.hpp"

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param server Server socket associated with connection
 * @param client Client socket associated with connection
 */
Connection::Connection(const Socket& server, const Socket& client)
	: m_serverSocket(server)
	, m_clientSocket(client)
	, m_timeSinceLastEvent(std::time(0))
	, m_status(ReceiveHeader)
	, m_bytesReceived(0)
{
}

void clearConnection(Connection& connection)
{
	connection.m_status = Connection::ReceiveHeader;
	connection.m_request = HTTPRequest();
	connection.m_buffer.clear();
	connection.m_bytesReceived = 0;
	connection.m_timeSinceLastEvent = std::time(0);
}
