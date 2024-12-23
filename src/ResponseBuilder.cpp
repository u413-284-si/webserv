#include "ResponseBuilder.hpp"
#include "StatusCode.hpp"
#include <cstddef>
#include <vector>

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

	HTTPRequest& request = connection.m_request;

	LOG_DEBUG << "Building response for request: " << request.method << " " << request.uri.path;

	ResponseBodyHandler responseBodyHandler(connection, m_responseBody, m_fileSystemPolicy);
	responseBodyHandler.execute();
	parseResponseBody(request);
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
 * The string m_responseBody is cleared as well as the map m_responseHeaders.
 */
void ResponseBuilder::resetBuilder()
{
	m_responseHeader.str(std::string());
	m_responseHeader.clear();
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
		if (m_responseHeaders.find("Content-Type") != m_responseHeaders.end()) {
			m_responseHeader << "Content-Type: " << m_responseHeaders.at("Content-Type") << "\r\n";
			m_responseHeaders.erase("Content-Type");
		}
		else
			m_responseHeader << "Content-Type: " << getMIMEType(webutils::getFileExtension(request.targetResource))
							 << "\r\n";
		// Content-Length
		if (m_responseHeaders.find("Content-Length") != m_responseHeaders.end()) {
			m_responseHeader << "Content-Length: " << m_responseHeaders.at("Content-Length") << "\r\n";
			m_responseHeaders.erase("Content-Length");
		}
		else
			m_responseHeader << "Content-Length: " << m_responseBody.length() << "\r\n";
	}

	// Various headers from response
	for (std::map<std::string, std::string>::const_iterator iter = m_responseHeaders.begin();
		 iter != m_responseHeaders.end(); ++iter)
		m_responseHeader << iter->first << ": " << iter->second << "\r\n";
	
	// Server
	m_responseHeader << "Server: TriHard\r\n";

	// Date
	m_responseHeader << "Date: " << webutils::getGMTString(time(0), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";

	// Location
	std::map<std::string, std::string>::const_iterator iter = request.headers.find("location");
	if (iter != request.headers.end()) {
		m_responseHeader << "Location: " << iter->second << "\r\n";
	}

	// Connection
	if (request.shallCloseConnection)
		m_responseHeader << "Connection: close\r\n";
	else
		m_responseHeader << "Connection: keep-alive\r\n";
	
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

void ResponseBuilder::parseResponseBody(HTTPRequest& request)
{
	if (m_responseBody.empty())
		return;

	LOG_DEBUG << "Parsing received CGI response body...";

	parseResponseStatusLine(request);
	parseResponseHeaders();
	processResponseHeaders(request);
}

/**
 * @brief Parses the response status line from the HTTP response body, if given.
 * 
 * This function searches for the status line in the HTTP response body and extracts
 * the HTTP status code. It updates the HTTPRequest object with the parsed status code.
 * 
 * @param request The HTTPRequest object to be updated with the parsed status code.
 */
void ResponseBuilder::parseResponseStatusLine(HTTPRequest& request)
{
	size_t posStatusEnd = 0;
	std::string httpString = "HTTP/1.1";
	size_t httpStringSize = httpString.size();
	std::vector<std::string> statusIdentifiers;
	statusIdentifiers.push_back("Status");
	statusIdentifiers.push_back(httpString);

	for (std::vector<std::string>::const_iterator iter = statusIdentifiers.begin(); iter != statusIdentifiers.end();
		 ++iter) {
		size_t posStatus = m_responseBody.find(*iter);

		if (posStatus != std::string::npos) {
			std::string statusLine;

			if (*iter == httpString)
				posStatus += httpStringSize;

			posStatusEnd = m_responseBody.find("\r\n", posStatus);
			statusLine = m_responseBody.substr(posStatus, posStatusEnd - posStatus);
			request.httpStatus = webutils::extractStatusCode(statusLine);
			LOG_DEBUG << "Parsed response status: " << request.httpStatus;
			posStatusEnd += 2;
		}
	}

	if (posStatusEnd != 0)
		m_responseBody = m_responseBody.substr(posStatusEnd);
}

/**
 * @brief Parses the response headers from the HTTP response body.
 * 
 * This function searches for the headers in the HTTP response body and extracts
 * them into a map of header names and values. It updates the response headers map
 * with the parsed headers.
 */
void ResponseBuilder::parseResponseHeaders()
{
	size_t posHeadersEnd = m_responseBody.find("\r\n\r\n");

	if (posHeadersEnd != std::string::npos) {
		// Include one CRLF at the end of last header line
		std::string headers = m_responseBody.substr(0, posHeadersEnd + 2);
		size_t lineStart = 0;
		size_t lineEnd = headers.find("\r\n");

		while (lineEnd != std::string::npos) {
			std::string header = headers.substr(lineStart, lineEnd - lineStart);

			std::string headerName;
			std::string headerValue;
			const std::size_t delimiterPos = header.find_first_of(':');
			if (delimiterPos != std::string::npos) {
				headerName = header.substr(0, delimiterPos);
				headerValue = header.substr(delimiterPos + 1);
				headerValue = webutils::trimLeadingWhitespaces(headerValue);
				webutils::trimTrailingWhiteSpaces(headerValue);
				m_responseHeaders[headerName] = headerValue;
				LOG_DEBUG << "Parsed response header: " << headerName << " -> " << headerValue;
			}

			lineStart = lineEnd + 2;
			lineEnd = headers.find("\r\n", lineStart);
		}

		m_responseBody = m_responseBody.substr(posHeadersEnd + 4);
	}
}

/**
 * @brief Processes the response headers and updates the HTTPRequest object.
 * 
 * This function processes the response headers and updates the HTTPRequest object
 * with relevant information, such as whether the connection should be closed.
 * 
 * @param request The HTTPRequest object to be updated with the processed headers.
 */
void ResponseBuilder::processResponseHeaders(HTTPRequest& request)
{
	// Connection
	if (m_responseHeaders.find("Connection") != m_responseHeaders.end()) {
		if (m_responseHeaders.at("Connection") == "close")
			request.shallCloseConnection = true;
		m_responseHeaders.erase("Connection");
	}
}
