#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "HTTPRequest.hpp"
#include "Socket.hpp"
#include "StatusCode.hpp"

#include <cassert>
#include <vector>

/**
 * @brief Overload << operator to append a Location.
 *
 * @param ostream The output stream.
 * @param location The Location object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const Location& location)
{
	ostream << "Path: " << location.path << '\n';
	ostream << "Root: " << location.root << '\n';
	ostream << "Alias: " << location.alias << '\n';
	ostream << "Indices: " << '\n';
	for (std::vector<std::string>::const_iterator it = location.indices.begin(); it != location.indices.end(); ++it)
		ostream << "  " << *it << '\n';
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.hasAutoindex << '\n';
	ostream << "Max body size: " << location.maxBodySize << '\n';
	ostream << "Error Page:\n";
	for (std::map<statusCode, std::string>::const_iterator it = location.errorPage.begin();
		 it != location.errorPage.end(); ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
	ostream << "Allowed methods:\n";
	ostream << "  GET: " << location.allowMethods[0] << '\n';
	ostream << "  POST: " << location.allowMethods[1] << '\n';
	ostream << "  DELETE: " << location.allowMethods[2] << '\n';
	ostream << "Returns: "
			<< "[" << location.returns.first << "]: " << location.returns.second << '\n';
	return ostream;
}

/**
 * @brief Overload << operator to append a ServerConfig.
 *
 * @param ostream The output stream.
 * @param configServer The configServer object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer)
{
	ostream << "Server name: " << configServer.serverName << '\n';
	ostream << "Host: " << configServer.host << '\n';
	ostream << "Port: " << configServer.port << '\n';
	ostream << "Root: " << configServer.root << '\n';
	ostream << "Max body size: " << configServer.maxBodySize << '\n';
	ostream << "Error pages:\n";
	for (std::map<statusCode, std::string>::const_iterator it = configServer.errorPage.begin();
		 it != configServer.errorPage.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Locations:\n";
	for (std::vector<Location>::const_iterator it = configServer.locations.begin(); it != configServer.locations.end();
		 ++it)
		ostream << *it;
	return ostream;
}

/**
 * @brief Overload << operator to append a ConfigFile.
 *
 * @param ostream The output stream.
 * @param configFile The ConfigFile object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile)
{
	ostream << "Config file:\n";
	for (std::vector<ConfigServer>::const_iterator it = configFile.servers.begin(); it != configFile.servers.end();
		 ++it)
		ostream << *it;
	return ostream;
}

/**
 * @brief Overload << operator to a Method.
 *
 * Translates the Method enum to a string.
 * Since Method is an enum it should only contain valid values. On the off chance of memory bugs, it's asserted that the
 * value is in range of the enum.
 * If the Method is MethodCount, it will output "enum MethodCount".
 * @param ostream The output stream.
 * @param method The Method enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, Method method)
{
	assert(method >= MethodGet && method <= MethodCount);

	switch (method) {
	case MethodGet:
		ostream << "GET";
		break;
	case MethodPost:
		ostream << "POST";
		break;
	case MethodDelete:
		ostream << "DELETE";
		break;
	case MethodCount:
		ostream << "enum MethodCount";
		break;
	}
	return ostream;
}

/**
 * @brief Overload << operator to append a statusCode.
 *
 * Translates the enum statusCode to a string representing the number.
 * Since Method is an enum it should only contain valid values. On the off chance of memory bugs, it's asserted that the
 * value is in range of the enum.
 * In case of NoStatus the string "0" is returned, which is not a valid/used status code.
 *
 * @param ostream The output stream.
 * @param statusCode The statusCode enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, statusCode statusCode)
{
	ostream << statusCodeToString(statusCode);
	return ostream;
}

/**
 * @brief Overload << operator to append a URI.
 *
 * @param ostream The output stream.
 * @param uri The URI object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const URI& uri)
{
	ostream << "  Path: " << uri.path << '\n';
	ostream << "  Query: " << uri.query << '\n';
	ostream << "  Fragment: " << uri.fragment << '\n';
	return ostream;
}

/**
 * @brief Overload << operator to append a HTTPRequest.
 *
 * @param ostream The output stream.
 * @param httpRequest The HTTPRequest object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const HTTPRequest& httpRequest)
{
	ostream << "Method: " << httpRequest.method << '\n';
	ostream << "URI:\n" << httpRequest.uri;
	ostream << "Version: " << httpRequest.version << '\n';
	ostream << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = httpRequest.headers.begin();
		 it != httpRequest.headers.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Body: " << httpRequest.body << '\n';
	ostream << "HTTP status: " << httpRequest.httpStatus << '\n';
	ostream << "Shall close connection: " << httpRequest.shallCloseConnection << '\n';
	ostream << "Has body: " << httpRequest.hasBody << '\n';
	ostream << "Is chunked: " << httpRequest.isChunked << '\n';
	ostream << "Target resource: " << httpRequest.targetResource << '\n';
	ostream << "Is directory: " << httpRequest.isDirectory << '\n';
	ostream << "Has autoindex: " << httpRequest.hasAutoindex << '\n';
	ostream << "Has CGI: " << httpRequest.hasCGI << '\n';
	return ostream;
}

/**
 * @brief Overload << operator to append a Socket.
 *
 * @param ostream The output stream.
 * @param socket The Socket object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const Socket& socket)
{
	ostream << socket.host << ':' << socket.port;
	return ostream;
}

/**
 * @brief Overload << operator to append a Connection::ConnectionStatus.
 *
 * @param ostream The output stream.
 * @param status The Connection::ConnectionStatus enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const Connection::ConnectionStatus status)
{
	assert(status >= Connection::Idle && status <= Connection::Closed);

	switch (status) {
	case Connection::Idle:
		ostream << "Idle";
		break;
	case Connection::ReceiveHeader:
		ostream << "ReceiveHeader";
		break;
	case Connection::ReceiveBody:
		ostream << "ReceiveBody";
		break;
	case Connection::SendToCGI:
		ostream << "SendToCGI";
		break;
	case Connection::ReceiveFromCGI:
		ostream << "ReceiveFromCGI";
		break;
	case Connection::BuildResponse:
		ostream << "BuildResponse";
		break;
	case Connection::SendResponse:
		ostream << "SendResponse";
		break;
	case Connection::Timeout:
		ostream << "Timeout";
		break;
	case Connection::Closed:
		ostream << "Closed";
		break;
	}
	return ostream;
}

/**
 * @brief Overload << operator to append a Connection.
 *
 * @param ostream The output stream.
 * @param connection The Connection object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const Connection& connection)
{
	ostream << "Server: " << connection.m_serverSocket << '\n';
	ostream << "Client: " << connection.m_clientSocket << '\n';
	ostream << "Time since last event: " << connection.m_timeSinceLastEvent << '\n';
	ostream << "Status: " << connection.m_status << '\n';
	ostream << "Request:\n" << connection.m_request;
	ostream << "Buffer:\n" << connection.m_buffer << '\n';
	return ostream;
}
