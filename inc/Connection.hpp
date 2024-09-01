#pragma once

#include "HTTPRequest.hpp"
#include "Socket.hpp"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <string>
#include <unistd.h>

/**
 * @brief Represents a connection between a server and a client.
 */
struct Connection {
public:
	Connection();
	Connection(const Socket& server, const Socket& client);

	enum ConnectionStatus {
		Idle, /**< Connection is connected, but no action is taken yet */
		ReceiveHeader, /**< Client wants to send request header */
		ReceiveBody, /**< Client wants to send request body */
		BuildResponse, /**< Full request received, Server can build response */
		SendResponse, /**< Server sends response */
		Timeout, /**< Timeout was reached after nothing happened in connection */
		Closed /**< Connection resources can be released */
		};

	Socket m_serverSocket; /**< Server socket associated with connection */
	Socket m_clientSocket; /**< Client socket associated with connection */
	time_t m_timeSinceLastEvent; /**< Time elapsed since last action on this connection */
	ConnectionStatus m_status; /**< Current status of the connection */
	std::string m_buffer; /**< Bytes received from client */
	ssize_t m_bytesReceived; /**< Number of bytes received from client */
	HTTPRequest m_request; /**< Request of the client */
};

void clearConnection(Connection& connection);

std::ostream& operator<<(std::ostream& ostream, const Connection& connection);
std::ostream& operator<<(std::ostream& ostream, const Connection::ConnectionStatus status);
