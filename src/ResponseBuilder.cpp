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

	setHeaderForStatusCode(request, connection.location->allowedMethods);

	appendResponseHeader(request);

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
 * @brief Appends the response status line and headers to the response header stream.
 *
 * This function constructs and appends the necessary HTTP status line and response headers to the response header
 * stream based on the provided HTTP request and the current state of the response.
 *
 * @param request The HTTP request object containing the request details.
 */
void ResponseBuilder::appendResponseHeader(const HTTPRequest& request)
{
	if (!checkForExistingHeader("status"))
		m_responseHeaderStream << "HTTP/1.1 " << request.httpStatus << ' '
							   << statusCodeToReasonPhrase(request.httpStatus) << "\r\n";

	if (!m_responseBody.empty()) {
		// Content-Type
		if (!checkForExistingHeader("content-type"))
			m_responseHeaderStream << "Content-Type: "
								   << getMIMEType(webutils::getFileExtension(request.targetResource)) << "\r\n";
		// Content-Length
		if (!checkForExistingHeader("content-length"))
			m_responseHeaderStream << "Content-Length: " << m_responseBody.length() << "\r\n";
	}

	// Server
	if (!checkForExistingHeader("server"))
		m_responseHeaderStream << "Server: TriHard\r\n";

	// Date
	if (!checkForExistingHeader("date"))
		m_responseHeaderStream << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";

	// Location
	checkForExistingHeader("location");

	// Various headers from response
	for (std::map<std::string, std::string>::const_iterator iter = m_responseHeaders.begin();
		 iter != m_responseHeaders.end(); ++iter)
		m_responseHeaderStream << webutils::capitalizeWords(iter->first) << ": " << iter->second << "\r\n";

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
bool ResponseBuilder::checkForExistingHeader(const std::string& headerName)
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

/**
 * @brief Sets specific headers depending on status code of HTTP request.
 *
 * For redirection status (3xx) adds location header.
 * For Method Not Allowed (405) adds allow header.
 * @param request The HTTP request object containing the request details.
 * @param allowedMethods Array of allowed methods.
 */
void ResponseBuilder::setHeaderForStatusCode(const HTTPRequest& request, const bool (&allowedMethods)[MethodCount])
{
	if (isRedirectionStatus(request.httpStatus))
		m_responseHeaders["location"] = request.targetResource;

	if (request.httpStatus == StatusMethodNotAllowed)
		m_responseHeaders["allow"] = constructAllowHeader(allowedMethods);
}

/**
 * @brief Construct Allow header.
 *
 * Constructs the Allow header based on the allowed methods. Methods are appended with ", " at the end to easily join
 * them. If at the end at least one method was appended, the last ", " is removed.
 * If no methods were appended, an empty string is returned.
 * @param allowedMethods Array of allowed methods.
 * @return std::string Constructed Allow header.
 */
std::string constructAllowHeader(const bool (&allowedMethods)[MethodCount])
{
	std::string allowHeader;

	if (allowedMethods[MethodGet])
		allowHeader.append("GET, ");
	if (allowedMethods[MethodPost])
		allowHeader.append("POST, ");
	if (allowedMethods[MethodDelete])
		allowHeader.append("DELETE, ");

	if (!allowHeader.empty())
		allowHeader.erase(allowHeader.size() - 2, 2);
	return allowHeader;
}
