#pragma once

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Socket.hpp"
#include "utilities.hpp"

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
	Connection(const Socket& server, const Socket& client, int clientFd, const std::vector<ConfigServer>& serverConfigs);

	enum ConnectionStatus {
        Idle, /**< Connection is connected, but no action is taken yet */
		ReceiveHeader, /**< Client wants to send request header */
		ReceiveBody, /**< Client wants to send request body */
		SendToCGI, /**< Received body is being sent to CGI */
		ReceiveFromCGI, /**< Message from CGI is being received */
		BuildResponse, /**< Full request received, Server can build response */
		SendResponse, /**< Server sends response */
		Timeout, /**< Timeout was reached after nothing happened in connection */
		Closed /**< Connection resources can be released */
	};

	Socket m_serverSocket; /**< Server socket associated with connection */
	Socket m_clientSocket; /**< Client socket associated with connection */
	int m_clientFd; /**< File descriptor of the client socket */
	time_t m_timeSinceLastEvent; /**< Time elapsed since last action on this connection */
	ConnectionStatus m_status; /**< Current status of the connection */
	std::string m_buffer; /**< Bytes received from client */
	HTTPRequest m_request; /**< Request of the client */
	std::vector<ConfigServer>::const_iterator serverConfig; /**< Server configuration associated with connection */
	std::vector<Location>::const_iterator location; /**< Location configuration associated with connection */
	int m_pipeToCGIWriteEnd; /**< Write end of the pipe to the CGI process */
	int m_pipeFromCGIReadEnd; /**< Read end of the pipe to the CGI process */
	pid_t m_cgiPid; /**< Process ID of the CGI process */
};

bool clearConnection(Connection& connection, const std::vector<ConfigServer>& serverConfigs);

bool hasValidServerConfig(Connection& connection, const std::vector<ConfigServer>& serverConfigs);
bool hasValidServerConfig(
	Connection& connection, const std::vector<ConfigServer>& serverConfigs, const std::string& host);

std::ostream& operator<<(std::ostream& ostream, const Connection& connection);
std::ostream& operator<<(std::ostream& ostream, Connection::ConnectionStatus status);
