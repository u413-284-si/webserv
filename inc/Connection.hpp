#pragma once

#include "Socket.hpp"
#include <cstddef>
#include <ctime>
#include <unistd.h>

/**
 * @brief Represents a connection between a server and a client.
 */
class Connection {
public:
	Connection();
	Connection(const Socket& server, const Socket& client);

	enum ConnectionStatus { ReceiveRequest, ReceiveBody, BuildResponse, SendResponse, Timeout };

	void updateTimeSinceLastEvent();
	void setStatus(ConnectionStatus status);
	void updateBytesReceived(std::size_t bytesReceived);

	Socket getServerSocket() const;
	Socket getClientSocket() const;
	time_t getTimeSinceLastEvent() const;
	ConnectionStatus getStatus() const;
	std::size_t getBytesReceived() const;

private:
	Socket m_serverSocket; /**< Server socket associated with connection */
	Socket m_clientSocket; /**< Client socket associated with connection */
	time_t m_timeSinceLastEvent; /**< Time elapsed since last action on this connection */
	ConnectionStatus m_status; /**< Current status of the connection */
	std::size_t m_bytesReceived; /**< Number of bytes received from client */
};
