#include "ResponseBuilder.hpp"

/**
 * @brief Construct a new ResponseBuilder object
 *
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBuilder::ResponseBuilder(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
	, m_isFirstTime(true)
{
	initMIMETypes();
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
 * Execute response body handler to get the response body.
 * Build the response by appending the status line, headers and body.
 * @param request HTTP request.
 */
void ResponseBuilder::buildResponse(HTTPRequest& request)
{
	resetStream();

	LOG_DEBUG << "Building response for request: " << request.method << " " << request.uri.path;

	ResponseBodyHandler responseBodyHandler(request, m_responseBody, m_fileSystemPolicy);
	responseBodyHandler.execute();

	LOG_DEBUG << "Response body: \n" << m_responseBody;

	appendStatusLine(request);
	appendHeaders(request);
	m_responseStream << m_responseBody << "\r\n";
	m_responseBody.clear();
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
 * @brief Append status line to the response.
 *
 * The status line is appended in the following format:
 * HTTP/1.1 <status code> <reason phrase>
 * @param request HTTP request.
 */
void ResponseBuilder::appendStatusLine(const HTTPRequest& request)
{
	m_responseStream << "HTTP/1.1 " << request.httpStatus << ' '
					 << webutils::statusCodeToReasonPhrase(request.httpStatus) << "\r\n";
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
 * @param request HTTP request.
 */
void ResponseBuilder::appendHeaders(const HTTPRequest& request)
{
	// Content-Type
	m_responseStream << "Content-Type: " << getMIMEType(webutils::getFileExtension(request.targetResource)) << "\r\n";
	// Content-Length
	m_responseStream << "Content-Length: " << m_responseBody.length() << "\r\n";
	// Server
	m_responseStream << "Server: TriHard\r\n";
	// Date
	m_responseStream << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	// Location
	if (request.httpStatus == StatusMovedPermanently) {
		m_responseStream << "Location: " << request.targetResource << "\r\n";
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
