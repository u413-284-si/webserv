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
		return m_responseHeader.str();

	return m_responseHeader.str() + m_responseBody;
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

	const HTTPRequest& request = connection.m_request;

	LOG_DEBUG << "Building response for request: " << request.method << " " << request.uri.path;

	ResponseBodyHandler responseBodyHandler(connection, m_responseBody, m_fileSystemPolicy);
	responseBodyHandler.execute();

	appendStatusLine(request);
	appendHeaders(request);

	LOG_DEBUG << "Response header: \n" << m_responseHeader.str();

	// Response Body often contains binary data, so it is not logged.
	// LOG_DEBUG << "Response body: \n" << m_responseBody;
}

/**
 * @brief Reset the builder.
 *
 * The stringstream m_responseHeader is reset to an empty string and cleared to removed any potential error flags.
 * The string m_responseBody is cleared.
 */
void ResponseBuilder::resetBuilder()
{
	m_responseHeader.str(std::string());
	m_responseHeader.clear();
	m_responseBody.clear();
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
	m_responseHeader << "HTTP/1.1 " << request.httpStatus << ' '
					 << webutils::statusCodeToReasonPhrase(request.httpStatus) << "\r\n";
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
	m_responseHeader << "Content-Type: " << getMIMEType(webutils::getFileExtension(request.targetResource)) << "\r\n";
	// Content-Length
	m_responseHeader << "Content-Length: " << m_responseBody.length() << "\r\n";
	}
	// Server
	m_responseHeader << "Server: TriHard\r\n";
	// Date
	m_responseHeader << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	// Location
	if (request.httpStatus == StatusMovedPermanently) {
		m_responseHeader << "Location: " << request.targetResource << "\r\n";
	}
	// Delimiter
	m_responseHeader << "\r\n";
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
