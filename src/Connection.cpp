#include "Connection.hpp"

/**
 * @brief Construct a new Connection:: Connection object
 *
 * The time since last event is initialized to the current time.
 * The status is initialized to ReceiveRequest.
 * Default constructor is needed for STL containers.
 * For example std::map::[] operator requires default constructor.
 */
Connection::Connection()
	: m_serverSocket(Socket())
	, m_clientSocket(Socket())
	, m_timeSinceLastEvent(std::time(0))
	, m_status(ReceiveRequest)
{
}

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
	, m_status(ReceiveRequest)
{
}
