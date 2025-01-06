#include "ResponseBuilder.hpp"

/**
 * @brief Construct a new ResponseBuilder object
 *
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBuilder::ResponseBuilder(const FileSystemPolicy& fileSystemPolicy, std::map<std::string, std::string>& responseHeaders)
	: m_fileSystemPolicy(fileSystemPolicy)
	, m_responseHeaders(responseHeaders)
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
 * @brief Append CGI headers to the response.
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
	if (!checkForCGIHeader("status"))
		m_responseHeaderStream << "HTTP/1.1 " << request.httpStatus << ' ' << statusCodeToReasonPhrase(request.httpStatus)
							   << "\r\n";

	if (!m_responseBody.empty()) {
		// Content-Type
		if (!checkForCGIHeader("content-type"))
			m_responseHeaderStream << "Content-Type: "
								   << getMIMEType(webutils::getFileExtension(request.targetResource)) << "\r\n";
		// Content-Length
		if (!checkForCGIHeader("content-length"))
			m_responseHeaderStream << "Content-Length: " << m_responseBody.length() << "\r\n";
	}

	// Server
	if (!checkForCGIHeader("server"))
		m_responseHeaderStream << "Server: TriHard\r\n";

	// Date
	if (!checkForCGIHeader("date"))
		m_responseHeaderStream << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";

	// Location
	checkForCGIHeader("location");

	// Various headers from response
	if (request.httpStatus < StatusMovedPermanently) {
		for (std::map<std::string, std::string>::const_iterator iter = m_responseHeaders.begin();
			 iter != m_responseHeaders.end(); ++iter)
			m_responseHeaderStream << webutils::capitalizeWords(iter->first) << ": " << iter->second << "\r\n";
	}

	// Connection
	if (request.shallCloseConnection)
		m_responseHeaderStream << "Connection: close\r\n";
	else
		m_responseHeaderStream << "Connection: keep-alive\r\n";

	// Delimiter
	m_responseHeaderStream << "\r\n";
}

/**
 * @brief Checks if the specified CGI header exists in the response headers.
 *
 * This function searches for the specified header name in the response headers.
 * If the header is found, it appends the header and its value to the response
 * header stream, removes the header from the response headers map, and returns true.
 * If the header is not found, it returns false.
 *
 * @param headerName The name of the header to search for.
 * @return true if the header is found and processed, false otherwise.
 */
bool ResponseBuilder::checkForCGIHeader(const std::string& headerName)
{
	std::map<std::string, std::string>::iterator iter = m_responseHeaders.find(headerName);
	if (iter != m_responseHeaders.end()) {
		if (iter->first == "status")
			m_responseHeaderStream << "HTTP/1.1 " << iter->second << "\r\n";
		else
			m_responseHeaderStream << webutils::capitalizeWords(iter->first) << ": " << iter->second << "\r\n";
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
	m_mimeTypes["json"] = "application/json";
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
