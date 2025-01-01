#include "ResponseBuilder.hpp"

/**
 * @brief Construct a new ResponseBuilder object
 *
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBuilder::ResponseBuilder(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
	initMIMETypes();
}

/**
 * @brief Get the built response.
 *
 * If the response body is empty, only the response header is returned.
 * Otherwise, the response header and body are returned.
 * The response doesn't change until the next buildResponse() call.
 * @return std::string Response.
 */
std::string ResponseBuilder::getResponse() const
{
	if (m_responseBody.empty())
		return m_responseHeaderStream.str();

	return m_responseHeaderStream.str() + m_responseBody;
}

/**
 * @brief Build the response for a given request.
 *
 * Reset the builder with resetBuilder(), to clear possible beforehand built response.
 * Construct and execute a ResponseBodyHandler to construct the response body.
 * Build the response header with appendStatusLine() and appendHeaders().
 * The response is stored in m_responseHeader and m_responseBody. The response can be retrieved with getResponse().
 * @param connection The connection to build the response for.
 */
void ResponseBuilder::buildResponse(Connection& connection)
{
	resetBuilder();

	HTTPRequest& request = connection.m_request;

	LOG_DEBUG << "Building response for request: " << request.method << " " << request.uri.path;

	ResponseBodyHandler responseBodyHandler(connection, m_responseBody, m_responseHeaders, m_fileSystemPolicy);
	responseBodyHandler.execute();
	appendStatusLine(request);
	appendHeaders(request);

	LOG_DEBUG << "Response header: \n" << m_responseHeaderStream.str();

	// Response Body often contains binary data, so it is not logged.
	// LOG_DEBUG << "Response body: \n" << m_responseBody;
}

/**
 * @brief Reset the builder.
 *
 * The stringstream m_responseHeader is reset to an empty string and cleared to removed any potential error flags.
 * The string m_responseBody is cleared as well as the map m_responseHeaders.
 */
void ResponseBuilder::resetBuilder()
{
	m_responseHeaderStream.str(std::string());
	m_responseHeaderStream.clear();
	m_responseBody.clear();
	m_responseHeaders.clear();
}

/**
 * @brief Append status line to the response.
 *
 * The status line is appended in the following format:
 * HTTP/1.1 <status code> <reason phrase> CRLF
 * @param request HTTP request.
 */
void ResponseBuilder::appendStatusLine(const HTTPRequest& request)
{
	m_responseHeaderStream << "HTTP/1.1 " << request.httpStatus << ' ' << statusCodeToReasonPhrase(request.httpStatus)
						   << "\r\n";
}

/**
 * @brief Append headers to the response.
 *
 * The following headers are appended:
 * - Content-Type: MIME type of the target resource (only if response has body)
 * - Content-Length: Length of the response body (only if response has body)
 * - Server: TriHard.
 * - Date: Current date in GMT.
 * - Location: Target resource if status is StatusMovedPermanently.
 * Delimiter.
 * @param request HTTP request.
 */
void ResponseBuilder::appendHeaders(const HTTPRequest& request)
{
	if (!m_responseBody.empty()) {
		// Content-Type
		if (!isCGIHeader("Content-Type"))
            m_responseHeaderStream << "Content-Type: "
								   << getMIMEType(webutils::getFileExtension(request.targetResource)) << "\r\n";
		// Content-Length
        if (!isCGIHeader("Content-Length"))
			m_responseHeaderStream << "Content-Length: " << m_responseBody.length() << "\r\n";
	}

	// Various headers from response
	for (std::map<std::string, std::string>::const_iterator iter = m_responseHeaders.begin();
		 iter != m_responseHeaders.end(); ++iter)
		m_responseHeaderStream << iter->first << ": " << iter->second << "\r\n";

	// Server
	if (!isCGIHeader("Server"))
		m_responseHeaderStream << "Server: TriHard\r\n";

	// Date
    if (!isCGIHeader("Date"))
	    m_responseHeaderStream << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";

	// Location
    if (!isCGIHeader("Location")) {
        std::map<std::string, std::string>::const_iterator iter = request.headers.find("location");
        if (iter != request.headers.end()) {
            m_responseHeaderStream << "Location: " << iter->second << "\r\n";
        }
    }

	// Connection
	if (request.shallCloseConnection)
		m_responseHeaderStream << "Connection: close\r\n";
	else
		m_responseHeaderStream << "Connection: keep-alive\r\n";

	// Delimiter
	m_responseHeaderStream << "\r\n";
}

bool ResponseBuilder::isCGIHeader(const std::string& headerName)
{
	std::map<std::string, std::string>::iterator iter = m_responseHeaders.find(headerName);
	if (iter != m_responseHeaders.end()) {
		m_responseHeaderStream << iter->first << ": " << iter->second << "\r\n";
		m_responseHeaders.erase(iter);
		return true;
	}
	return false;
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
