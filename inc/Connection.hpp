#pragma once

#include <cstddef>
#include <cstdio>
#include <ctime>
#include <string>
#include <unistd.h>

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Socket.hpp"

/**
 * @brief Represents a connection between a server and a client.
 */
struct Connection {
public:
	Connection(const Socket& server, const Socket& client, const std::vector<ConfigServer>& serverConfigs);

	enum ConnectionStatus { ReceiveHeader, ReceiveBody, BuildResponse, SendResponse, Timeout, Closed };

	Socket m_serverSocket; /**< Server socket associated with connection */
	Socket m_clientSocket; /**< Client socket associated with connection */
	time_t m_timeSinceLastEvent; /**< Time elapsed since last action on this connection */
	ConnectionStatus m_status; /**< Current status of the connection */
	std::string m_buffer; /**< Bytes received from client */
	ssize_t m_bytesReceived; /**< Number of bytes received from client */
	HTTPRequest m_request; /**< Request of the client */
	std::vector<ConfigServer>::const_iterator serverConfig; /**< Server configuration associated with connection */
	std::vector<Location>::const_iterator location; /**< Location configuration associated with connection */
};

bool clearConnection(Connection& connection, const std::vector<ConfigServer>& serverConfigs);

bool hasValidServerConfig(Connection& connection, const std::vector<ConfigServer>& serverConfigs);
bool hasValidServerConfig(
	Connection& connection, const std::vector<ConfigServer>& serverConfigs, const std::string& host);

std::ostream& operator<<(std::ostream& ostream, const Connection& connection);
std::ostream& operator<<(std::ostream& ostream, Connection::ConnectionStatus status);
