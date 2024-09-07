#pragma once

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Socket.hpp"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <sched.h>
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
		ReceiveHeader,
		ReceiveBody,
		SendToCGI,
		ReceiveFromCGI,
		BuildResponse,
		SendResponse,
		Timeout,
		Closed
	};

	Socket m_serverSocket; /**< Server socket associated with connection */
	Socket m_clientSocket; /**< Client socket associated with connection */
	time_t m_timeSinceLastEvent; /**< Time elapsed since last action on this connection */
	ConnectionStatus m_status; /**< Current status of the connection */
	std::string m_buffer; /**< Bytes received from client */
	ssize_t m_bytesReceived; /**< Number of bytes received from client */
	HTTPRequest m_request; /**< Request of the client */
	HTTPResponse m_response; /**< Response to be sent to the client */
	int m_pipeToCGIWriteEnd; /**< Write end of the pipe to the CGI process */
	int m_pipeFromCGIReadEnd; /**< Read end of the pipe to the CGI process */
    pid_t m_cgiPid; /**< Process ID of the CGI process */
};

void clearConnection(Connection& connection);

std::ostream& operator<<(std::ostream& ostream, const Connection& connection);
std::ostream& operator<<(std::ostream& ostream, const Connection::ConnectionStatus status);
