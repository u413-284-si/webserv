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
	: m_server(Socket())
	, m_client(Socket())
	, m_TimeSinceLastEvent(std::time(0))
	, m_status(ReceiveRequest)
	, m_bytesReceived(0)
{
}

/**
 * @brief Construct a new Connection:: Connection object
 *
 * @param server Server socket associated with connection
 * @param client Client socket associated with connection
 */
Connection::Connection(const Socket& server, const Socket& client)
	: m_server(server)
	, m_client(client)
	, m_TimeSinceLastEvent(std::time(0))
	, m_status(ReceiveRequest)
	, m_bytesReceived(0)
{
}

/**
 * @brief Close the client socket.
 * @todo Implement a more robust way to close the connection. For example in case of timeout
 *       the server send a message before closing the connection.
 */
void Connection::closeConnection() const { close(m_client.fd); }

/**
 * @brief Update the time since the last event on this connection.
 */
void Connection::updateTimeSinceLastEvent() { m_TimeSinceLastEvent = std::time(0); }

/**
 * @brief Set the status of the connection.
 *
 * @param status New status of the connection
 */
void Connection::setStatus(ConnectionStatus status) { m_status = status; }

/**
 * @brief Increments the number of bytes received from the client.
 *
 * @param bytesReceived Number of bytes received from the client
 */
void Connection::updateBytesReceived(std::size_t bytesReceived) { m_bytesReceived += bytesReceived; }

/**
 * @brief Returns the server socket associated with the connection.
 *
 * @return Socket Server socket associated with connection
 */
Socket Connection::getServer() const { return m_server; }

/**
 * @brief Returns the client socket associated with the connection.
 *
 * @return Socket Client socket associated with connection
 */
Socket Connection::getClient() const { return m_client; }

/**
 * @brief Returns the time elapsed since the last action on this connection.
 *
 * @return time_t Time elapsed since last action on this connection
 */
time_t Connection::getTimeSinceLastEvent() const { return (std::time(0) - m_TimeSinceLastEvent); }

/**
 * @brief Returns the current status of the connection.
 *
 * @return ConnectionStatus Current status of the connection
 */
Connection::ConnectionStatus Connection::getStatus() const { return m_status; }

/**
 * @brief Returns the number of bytes received from the client.
 *
 * @return std::size_t Number of bytes received from the client
 */
std::size_t Connection::getBytesReceived() const { return m_bytesReceived; }
