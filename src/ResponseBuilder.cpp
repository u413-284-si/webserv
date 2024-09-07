#include "ResponseBuilder.hpp"
#include "ConfigFile.hpp"
#include "HTTPResponse.hpp"
#include <cstddef>

/**
 * @brief Construct a new ResponseBuilder object
 *
 * @param configFile Configuration file.
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBuilder::ResponseBuilder(const ConfigFile& configFile, const FileSystemPolicy& fileSystemPolicy)
	: m_activeServer(configFile.servers.begin())
	, m_fileSystemPolicy(fileSystemPolicy)
	, m_isFirstTime(true)
{
	initMIMETypes();
}

/**
 * @brief Set the active server.
 *
 * The active server is used for location searching.
 * @param activeServer Server to be set as active.
 */
void ResponseBuilder::setActiveServer(const std::vector<ConfigServer>::const_iterator& activeServer)
{
	m_activeServer = activeServer;
}

/**
 * @brief Get the built response.
 *
 * The stream is returned as c_str() since it is reused.
 * The buffer is never cleared all the way, only reset to the beginning.
 * If str() would be returned, old parts of the stream would be included.
 * This works since every thing inserted into the string sets a new zero terminator.
 * @return std::string Response.
 */
std::string ResponseBuilder::getResponse() const { return m_responseStream.str().c_str(); } // NOLINT

/**
 * @brief Build the response for a given request.
 *
 * Reset the stream, to clear possible beforehand built response.
 * Init the HTTP Response object with data from the request.
 * If StatusCode is StatusOK: Execute target resource handler to get the target resource.
 * Else skip this step.
 * Execute response body handler to get the response body.
 * Build the response by appending the status line, headers and body.
 * @param request HTTP request.
 */
void ResponseBuilder::buildResponse(Connection& connection)
{
	resetStream();

	LOG_DEBUG << "Building response for request: " << connection.m_request.method << " "
			  << connection.m_request.uri.path;

	initHTTPResponse(connection.m_request, connection.m_response);

	if (connection.m_response.status == StatusOK) {
		TargetResourceHandler targetResourceHandler(
			m_activeServer->locations, connection.m_request, connection.m_response, m_fileSystemPolicy);
		targetResourceHandler.execute();
	}

	LOG_DEBUG << "Target resource: " << connection.m_response.targetResource;

	if (isCGIRequested(connection.m_request, connection.m_response))
		connection.m_response.isCGI = true;

	ResponseBodyHandler responseBodyHandler(connection.m_response, m_fileSystemPolicy);
	responseBodyHandler.execute(connection);

	LOG_DEBUG << "Response body: " << connection.m_response.body;

	appendStatusLine(connection.m_response);
	appendHeaders(connection.m_response);
	m_responseStream << connection.m_response.body << "\r\n";
}

/**
 * @brief Reset the stream to the beginning.
 *
 * If it is not the first time, the stream is reset to the beginning.
 * This is done to clear the stream from the previous response.
 */
void ResponseBuilder::resetStream()
{
	if (!m_isFirstTime)
		m_responseStream.seekp(std::ios::beg);

	m_isFirstTime = false;
}

/**
 * @brief Init the HTTP Response object with data from the request.
 *
 * @param request HTTP request.
 * @param response HTTP response.
 */
void ResponseBuilder::initHTTPResponse(const HTTPRequest& request, HTTPResponse& response)
{
	response.status = request.httpStatus;
	response.method = request.method;
	response.isAutoindex = false;
	response.isCGI = false;
}

/**
 * @brief Append status line to the response.
 *
 * The status line is appended in the following format:
 * HTTP/1.1 <status code> <reason phrase>
 * @param response HTTP response.
 */
void ResponseBuilder::appendStatusLine(const HTTPResponse& response)
{
	m_responseStream << "HTTP/1.1 " << response.status << ' ' << webutils::statusCodeToReasonPhrase(response.status)
					 << "\r\n";
}

/**
 * @brief Append headers to the response.
 *
 * The following headers are appended:
 * Content-Type: MIME type of the target resource.
 * Content-Length: Length of the response body.
 * Server: TriHard.
 * Date: Current date in GMT.
 * Location: Target resource if status is StatusMovedPermanently.
 * Delimiter.
 * @param response HTTP response.
 */
void ResponseBuilder::appendHeaders(const HTTPResponse& response)
{
	// Content-Type
	m_responseStream << "Content-Type: " << getMIMEType(webutils::getFileExtension(response.targetResource)) << "\r\n";
	// Content-Length
	m_responseStream << "Content-Length: " << response.body.length() << "\r\n";
	// Server
	m_responseStream << "Server: TriHard\r\n";
	// Date
	m_responseStream << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	// Location
	if (response.status == StatusMovedPermanently) {
		m_responseStream << "Location: " << response.targetResource << "\r\n";
	}
	// Delimiter
	m_responseStream << "\r\n";
}

/**
 * @brief Init the MIME types map.
 *
 * @todo FIXME: This needs to get the MIME type of the found location.
 */
void ResponseBuilder::initMIMETypes()
{
	m_mimeTypes["html"] = "text/html";
	m_mimeTypes["htm"] = "text/html";
	m_mimeTypes["jpg"] = "image/jpeg";
	m_mimeTypes["jpeg"] = "image/jpeg";
	m_mimeTypes["png"] = "image/png";
	m_mimeTypes["gif"] = "image/gif";
	m_mimeTypes["css"] = "text/css";
	m_mimeTypes["js"] = "application/javascript";
	m_mimeTypes["pdf"] = "application/pdf";
	m_mimeTypes["txt"] = "text/plain";
	m_mimeTypes["default"] = "application/octet-stream";
}

/**
 * @brief Returns the corresponding MIME type for a given extension.
 *
 * If the extension is not found in the map, the default MIME type is returned.
 * @param extension File extension.
 * @return std::string MIME type.
 * @todo FIXME: This needs to get the MIME type of the found location.
 */
std::string ResponseBuilder::getMIMEType(const std::string& extension)
{
	std::map<std::string, std::string>::const_iterator iter = m_mimeTypes.find(extension);
	if (iter != m_mimeTypes.end()) {
		return iter->second;
	}
	return m_mimeTypes.at("default");
}

/**
 * @brief Checks if CGI execution is requested for the given HTTP request and response.
 *
 * This function checks if CGI execution is requested based on the provided HTTP request and response.
 * It checks if the CGI path and extension are specified in the response location, and if the request URI path
 * contains the CGI path and extension. If the extension is found and is at the end of the path or followed by a slash,
 * it returns true indicating that CGI execution is requested. Otherwise, it returns false.
 *
 * @param request The HTTP request object.
 * @param response The HTTP response object.
 * @return true if CGI execution is requested, false otherwise.
 */
bool isCGIRequested(const HTTPRequest& request, const HTTPResponse& response)
{
	// FIXME: May need to iterate through multiple paths and extensions
	if (!response.location->cgiPath.empty() && request.uri.path.find(response.location->cgiPath) != std::string::npos
		&& !response.location->cgiExt.empty()) {
		size_t extPos = request.uri.path.find(response.location->cgiExt);

		// If extension is found and is at the end of the path or followed by a slash
		if (extPos != std::string::npos
			&& (extPos + response.location->cgiExt.size() == request.uri.path.size()
				|| request.uri.path.at(extPos + response.location->cgiExt.size()) == '/'))
			return true;
	}
	return false;
}
