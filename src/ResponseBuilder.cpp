#include "ResponseBuilder.hpp"

/**
 * @brief Construct a new ResponseBuilder object
 *
 * @param configFile Configuration file.
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBuilder::ResponseBuilder(const ConfigFile& configFile, const FileSystemPolicy& fileSystemPolicy)
	: m_configFile(configFile)
	, m_activeServer(configFile.serverConfigs.begin())
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
void ResponseBuilder::setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer)
{
	m_activeServer = activeServer;
}

/**
 * @brief Get the built response.
 *
 * The stream is returned as c_str() since it is reused.
 * If str() would be returned, old parts of the stream would be included.
 * @return std::string Response.
 */
std::string ResponseBuilder::getResponse() const { return m_response.str().c_str(); }

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
void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	resetStream();

	HTTPResponse response = initHTTPResponse(request);

	if (response.status != StatusOK) {
		TargetResourceHandler targetResourceHandler(m_activeServer->locations, request, response, m_fileSystemPolicy);
		targetResourceHandler.execute();
	}

	ResponseBodyHandler responseBodyHandler(response, m_fileSystemPolicy);
	responseBodyHandler.execute();

	appendStatusLine(response);
	appendHeaders(response);
	m_response << response.body << "\r\n";
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
		m_response.seekp(std::ios::beg);

	m_isFirstTime = false;
}

/**
 * @brief Init the HTTP Response object with data from the request.
 *
 * @param request HTTP request.
 * @return HTTPResponse Constructed HTTP response.
 */
HTTPResponse ResponseBuilder::initHTTPResponse(const HTTPRequest& request)
{
	HTTPResponse response;

	response.status = StatusOK;
	response.method = request.method;
	response.autoindex = false;

	return response;
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
	m_response << "HTTP/1.1 " << response.status << ' ' << statusCodeToReasonPhrase(response.status) << "\r\n";
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
	m_response << "Content-Type: " << getMIMEType(utils::getFileExtension(response.targetResource)) << "\r\n";
	// Content-Length
	m_response << "Content-Length: " << response.body.length() << "\r\n";
	// Server
	m_response << "Server: TriHard\r\n";
	// Date
	m_response << "Date: " << utils::getGMTString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	// Location
	if (response.status == StatusMovedPermanently) {
		m_response << "Location: " << response.targetResource << "\r\n";
	}
	// Delimiter
	m_response << "\r\n";
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
	std::map<std::basic_string<char>, std::basic_string<char> >::const_iterator iter = m_mimeTypes.find(extension);
	if (iter != m_mimeTypes.end()) {
		return iter->second;
	}
	return m_mimeTypes.at("default");
}
