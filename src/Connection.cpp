#include "Connection.hpp"
#include "ConfigFile.hpp"
#include "StatusCode.hpp"

/**
 * @brief Construct a new Connection:: Connection object
 *
 * The time since last event is initialized to the current time.
 * The status is initialized to ReceiveHeader.
 * Default constructor is needed for STL containers.
 * For example std::map::[] operator requires default constructor.
 */
Connection::Connection()
	: m_serverSocket(Socket())
	, m_clientSocket(Socket())
	, m_timeSinceLastEvent(std::time(0))
	, m_status(ReceiveHeader)
	, m_bytesReceived(0)
    , m_pipeToCGIWriteEnd(-1)
    , m_pipeFromCGIReadEnd(-1)
    , m_cgiPid(-1)
{
	m_request.method = MethodCount;
	m_request.httpStatus = StatusOK;
	m_request.shallCloseConnection = false;
	m_request.hasBody = false;
	m_request.isChunked = false;
	m_response.status = StatusOK;
	m_response.method = MethodCount;
	m_response.isAutoindex = false;
	m_response.isCGI = false;
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
	, m_status(ReceiveHeader)
	, m_bytesReceived(0)
    , m_pipeToCGIWriteEnd(-1)
    , m_pipeFromCGIReadEnd(-1)
    , m_cgiPid(-1)
    
{
	m_request.method = MethodCount;
	m_request.httpStatus = StatusOK;
	m_request.shallCloseConnection = false;
	m_request.hasBody = false;
	m_request.isChunked = false;
    m_response.status = StatusOK;
	m_response.method = MethodCount;
	m_response.isAutoindex = false;
	m_response.isCGI = false;
}

void clearConnection(Connection& connection)
{
	connection.m_status = Connection::ReceiveHeader;
	connection.m_request.method = MethodCount;
	connection.m_request.httpStatus = StatusOK;
	connection.m_request.shallCloseConnection = false;
	connection.m_request.hasBody = false;
	connection.m_request.isChunked = false;
    connection.m_response.status = StatusOK;
	connection.m_response.method = MethodCount;
	connection.m_response.isAutoindex = false;
	connection.m_response.isCGI = false;
	connection.m_buffer = "";
	connection.m_bytesReceived = 0;
    connection.m_pipeToCGIWriteEnd = -1;
    connection.m_pipeFromCGIReadEnd = -1;
    connection.m_cgiPid = -1;
	connection.m_timeSinceLastEvent = std::time(0);
}
