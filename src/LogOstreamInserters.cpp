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
	ostream << "Indices: " << '\n';
	for (std::vector<std::string>::const_iterator it = location.indices.begin(); it != location.indices.end(); ++it)
		ostream << "  " << *it << '\n';
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.isAutoindex << '\n';
	ostream << "Allowed methods:\n";
	ostream << "  GET: " << location.allowedMethods[0] << '\n';
	ostream << "  POST: " << location.allowedMethods[1] << '\n';
	ostream << "  DELETE: " << location.allowedMethods[2] << '\n';
	ostream << "Returns:\n";
	for (std::map<statusCode, std::string>::const_iterator it = location.returns.begin(); it != location.returns.end();
		 ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
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
 * Translates the Method enum to a string.
 * Since Method is an enum it should only contain valid values. On the off chance of memory bugs, it's asserted that the
 * value is in range of the enum.
 *
 * @param ostream The output stream.
 * @param statusCode The statusCode enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, statusCode statusCode)
{
	assert(statusCode >= StatusOK && statusCode <= StatusNonSupportedVersion);

	switch (statusCode) {
	case StatusOK:
		ostream << "200 OK";
		break;
	case StatusMovedPermanently:
		ostream << "301 Moved Permanently";
		break;
	case StatusBadRequest:
		ostream << "400 Bad Request";
		break;
	case StatusForbidden:
		ostream << "403 Forbidden";
		break;
	case StatusNotFound:
		ostream << "404 Not Found";
		break;
	case StatusMethodNotAllowed:
		ostream << "405 Method Not Allowed";
		break;
	case StatusRequestTimeout:
		ostream << "408 Request Timeout";
		break;
	case StatusRequestEntityTooLarge:
		ostream << "413 Request Entity Too Large";
		break;
	case StatusRequestHeaderFieldsTooLarge:
		ostream << "431 Request Header Fields Too Large";
		break;
	case StatusInternalServerError:
		ostream << "500 Internal Server Error";
		break;
	case StatusMethodNotImplemented:
		ostream << "501 Method Not Implemented";
		break;
	case StatusNonSupportedVersion:
		ostream << "505 HTTP Version Not Supported";
		break;
	}
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
