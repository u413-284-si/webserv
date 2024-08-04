#include "LogOstreamInserters.hpp"

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
	ostream << "Index: " << location.index << '\n';
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.isAutoindex << '\n';
	ostream << "LimitExcept:\n";
	ostream << "  Allowed methods:\n";
	for (int i = 0; i < MethodCount; ++i) {
		// NOLINTNEXTLINE: The bool array allowedMethods can never be out of bounds, since it has the same size as MethodCount.
		if (location.limitExcept.allowedMethods[i])
			ostream << "    " << static_cast<Method>(i) << '\n';
	}
	ostream << "  Allow: " << location.limitExcept.allow << '\n';
	ostream << "  Deny: " << location.limitExcept.deny << '\n';
	ostream << "Returns:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = location.returns.begin();
		 it != location.returns.end(); ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
	return ostream;
}

/**
 * @brief Overload << operator to append a ServerConfig.
 *
 * @param ostream The output stream.
 * @param serverConfig The ServerConfig object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const ServerConfig& serverConfig)
{
	ostream << "Server name: " << serverConfig.serverName << '\n';
	ostream << "Host: " << serverConfig.host << '\n';
	ostream << "Port: " << serverConfig.port << '\n';
	ostream << "Max body size: " << serverConfig.maxBodySize << '\n';
	ostream << "Error pages:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = serverConfig.errorPage.begin();
		 it != serverConfig.errorPage.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Locations:\n";
	for (std::vector<Location>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end();
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
	for (std::vector<ServerConfig>::const_iterator it = configFile.serverConfigs.begin();
		 it != configFile.serverConfigs.end(); ++it)
		ostream << *it;
	return ostream;
}

/**
 * @brief Overload << operator to a Method.
 *
 * Translates the Method enum to a string.
 * If the Method is MethodCount, it will output "enum MethodCount".
 * @param ostream The output stream.
 * @param method The Method enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, Method method)
{
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
 * @param ostream The output stream.
 * @param statusCode The statusCode enum.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, statusCode statusCode)
{
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
 * @brief Overload << operator to append a HTTPResponse.
 *
 * @param ostream The output stream.
 * @param httpResponse The HTTPResponse object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const HTTPResponse& httpResponse)
{
	ostream << "Status code: " << httpResponse.status << '\n';
	ostream << "Target resource: " << httpResponse.targetResource << '\n';
	ostream << "Body: " << httpResponse.body << '\n';
	ostream << "Location:\n" << *httpResponse.location << '\n';
	ostream << "Method: " << httpResponse.method << '\n';
	ostream << "Autoindex: " << httpResponse.isAutoindex << '\n';
	return ostream;
}
