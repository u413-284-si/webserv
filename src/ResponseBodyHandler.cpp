#include "ResponseBodyHandler.hpp"

/**
 * @brief Construct a new ResponseBodyHandler object
 *
 * @param connection The Connection for which the response body is handled.
 * @param responseBody Saves the response body.
 * @param responseHeaders Saves the response headers.
 * @param fileSystemOps Wrapper for filesystem-related functions. Can be mocked if needed.
 */
ResponseBodyHandler::ResponseBodyHandler(Connection& connection, std::string& responseBody,
	std::map<std::string, std::string>& responseHeaders, const FileSystemOps& fileSystemOps)
	: m_connection(connection)
	, m_request(connection.m_request)
	, m_responseBody(responseBody)
	, m_responseHeaders(responseHeaders)
	, m_fileSystemOps(fileSystemOps)
{
}

/**
 * @brief Create the response body based on the HTTP Request status and method.
 *
 * addHeadersBasedOnStatus() is called to add special headers based on the status code.
 * Then special cases are handled:
 * - If the request has a Return directive, handleReturnDirective() is called.
 * - If the status is an error status, handleErrorBody() is called.
 * - If the request has CGI, parseCGIResponseHeaders() is called.
 * Otherwise the method is checked and the appropriate function is called:
 * - GET: handleGetRequest()
 * - POST: handlePostRequest()
 * - DELETE: handleDeleteRequest()
 */
void ResponseBodyHandler::execute()
{
	addHeadersBasedOnStatus();

	if (m_request.hasReturn) {
		handleReturnDirective();
		return;
	}

	if (isErrorStatus(m_request.httpStatus)) {
		handleErrorBody();
		return;
	}

	if (m_request.hasCGI) {
		m_responseBody = m_request.body;
		parseCGIResponseHeaders();
		return;
	}

	switch (m_request.method) {
	case MethodGet:
		handleGetRequest();
		break;
	case MethodPost:
		handlePostRequest();
		break;
	case MethodDelete:
		handleDeleteRequest();
		break;
	case MethodCount:
		LOG_ERROR << "Invalid method";
		m_request.httpStatus = StatusInternalServerError;
		handleErrorBody();
		break;
	}
}

/**
 * @brief Creates the response body based on the Return directive.
 *
 * If the request encountered a location with a Return directive the body will be created based on the directive.
 * - If the target resource is empty and the status is not an error status, no body is sent.
 * - If the target resource is not empty and the status is not a redirection status, the target resource will be set as
 * the body.
 * - If the target resource is empty and the status is a redirection status, an error page will be created.
 */
void ResponseBodyHandler::handleReturnDirective()
{
	const bool isEmpty = m_request.targetResource.empty();

	if (isEmpty && !isErrorStatus(m_request.httpStatus))
		return;

	if (!isEmpty && !isRedirectionStatus(m_request.httpStatus))
		m_responseBody = m_request.targetResource;
	else
		handleErrorBody();
}

/**
 * @brief Creates the response body for a GET request.
 *
 * If the request has the autoindex flag set, an autoindex will be created. It also adds "autoindex.html" to the
 * target resource.
 * Otherwise the file contents of the target resource will be read and set as the body.
 */
void ResponseBodyHandler::handleGetRequest()
{
	LOG_DEBUG << "Handling GET request";

	if (m_request.hasAutoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemOps);
		m_responseBody = autoindexHandler.execute(m_request.targetResource, m_request.uri.path);
		if (m_responseBody.empty()) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
			return;
		}
		m_request.targetResource += "autoindex.html";
		return;
	}

	try {
		m_responseBody = m_fileSystemOps.getFileContents(m_request.targetResource.c_str());
	} catch (FileSystemOps::FileNotFoundException& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusNotFound;
	} catch (FileSystemOps::NoPermissionException& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusForbidden;
	} catch (const std::runtime_error& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusInternalServerError;
	}
	if (m_request.httpStatus != StatusOK)
		handleErrorBody();
}

/**
 * @brief Creates a file and the response body for a POST request.
 *
 * The request body will be written to the target resource. If the file was created / appended successfully, the
 * response body will be set to a JSON-formatted string containing the operation result. The target resource will be set
 * to "posted.json".
 */
void ResponseBodyHandler::handlePostRequest()
{
	LOG_DEBUG << "Handling POST request";

	FileWriteHandler fileWriteHandler(m_fileSystemOps);
	m_responseBody = fileWriteHandler.execute(m_request.targetResource, m_request.body, m_request.httpStatus);
	if (m_request.httpStatus == StatusCreated)
		m_responseHeaders["location"] = m_request.uri.path;
	if (m_responseBody.empty())
		handleErrorBody();
	else
		m_request.targetResource = "posted.json";
}

/**
 * @brief Deletes a file and creates the response body for a DELETE request.
 *
 * The target resource will be deleted. If the file was deleted successfully, the response body be set to a
 * JSON-formatted string containing the operation result. The target resource will be set to "deleted.json".
 */
void ResponseBodyHandler::handleDeleteRequest()
{
	LOG_DEBUG << "Handling DELETE request";

	DeleteHandler deleteHandler(m_fileSystemOps);
	m_responseBody = deleteHandler.execute(m_request.targetResource, m_request.httpStatus);
	if (m_responseBody.empty())
		handleErrorBody();
	else
		m_request.targetResource = "deleted.json";
}

/**
 * @brief Parses the response headers from the HTTP response body.
 *
 * This function searches for the headers in the HTTP response body and extracts
 * them into a map of header names and values. It updates the response headers map
 * with the parsed headers.
 */
void ResponseBodyHandler::parseCGIResponseHeaders()
{
	LOG_DEBUG << "Parsing received CGI response...";

	const size_t sizeCRLF = 2;
	const size_t sizeCRLFCRLF = 4;
	const size_t posHeadersEnd = m_responseBody.find("\r\n\r\n");
	// Include one CRLF at the end of last header line
	std::string headers = m_responseBody.substr(0, posHeadersEnd + sizeCRLF);
	const std::string loweredHeaders = webutils::lowercase(headers);

	if (posHeadersEnd == std::string::npos) {
		m_request.httpStatus = StatusInternalServerError;
		handleErrorBody();
		LOG_ERROR << ERR_MISSING_CGI_HEADER;
		return;
	}

	if (loweredHeaders.find("content-type: ") == std::string::npos
		&& loweredHeaders.find("location: ") == std::string::npos
		&& loweredHeaders.find("status: ") == std::string::npos) {
		m_request.httpStatus = StatusInternalServerError;
		handleErrorBody();
		LOG_ERROR << ERR_MISSING_CGI_FIELD;
		return;
	}

	size_t lineStart = 0;
	size_t lineEnd = headers.find("\r\n");

	while (lineEnd != std::string::npos) {
		std::string header = headers.substr(lineStart, lineEnd - lineStart);
		const std::size_t delimiterPos = header.find_first_of(':');
		if (delimiterPos != std::string::npos) {
			const std::string headerName = webutils::lowercase(header.substr(0, delimiterPos));
			std::string headerValue = header.substr(delimiterPos + 1);
			headerValue = webutils::trimWhiteSpaces(headerValue);
			m_responseHeaders[headerName] = headerValue;
			LOG_DEBUG << "Parsed response header: " << headerName << " -> " << headerValue;
		}

		lineStart = lineEnd + 2;
		lineEnd = headers.find("\r\n", lineStart);
	}
	m_responseBody.erase(0, posHeadersEnd + sizeCRLFCRLF);
	validateCGIResponseHeaders();
}

/**
 * @brief Processes and validates the response headers and updates the HTTPRequest object.
 *
 * This function processes the response headers and updates the HTTPRequest object
 * with relevant information, such as whether the connection should be closed.
 */
void ResponseBodyHandler::validateCGIResponseHeaders()
{
	// Status
	std::map<std::string, std::string>::iterator iter = m_responseHeaders.find("status");
	if (iter != m_responseHeaders.end()) {
		m_request.httpStatus = extractStatusCode(iter->second);
		if (m_request.httpStatus == NoStatus) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
			m_responseHeaders.clear();
			LOG_ERROR << "Invalid Status header value encountered in CGI response";
			return;
		}
	}

	// Connection
	iter = m_responseHeaders.find("connection");
	if (iter != m_responseHeaders.end()) {
		if (iter->second != "keep-alive" && iter->second != "close") {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
			m_responseHeaders.clear();
			LOG_ERROR << "Invalid Connection header value: " << iter->second;
			return;
		}

		if (iter->second == "close")
			m_request.shallCloseConnection = true;
		m_responseHeaders.erase(iter);
	}
}

/**
 * @brief Construct an error body.
 *
 * Checks if the active location has a custom error page for HTTP Status Code.
 * If yes tries to locate the error page via the provided URI.
 * - The current Status Code is saved and reset to StatusOK.
 * - The request hasReturn is set to false (in case it was set to true).
 * - The request.uri.path is set to the error page URI.
 * - Then tries to find it with a TargetResourceHandler.
 *
 * If the request hasReturn is set to true the status is set back to the saved one. Depending on whether the target
 * resource is empty error page is set via setDefaultErrorPage() or the return string is set as the error page.
 *
 * If no error happened while locating the error page indicated via Status == OK the error page is read into the
 * body and the status set back to the saved one.
 *
 * In case of any error a default error page is constructed via setDefaultErrorPage().
 */
void ResponseBodyHandler::handleErrorBody()
{
	LOG_DEBUG << "Create error body for " << m_request.httpStatus;

	std::map<statusCode, std::string>::const_iterator iter
		= m_connection.location->errorPage.find(m_request.httpStatus);

	if (iter == m_connection.location->errorPage.end()) {
		LOG_DEBUG << "No custom error page";
		setDefaultErrorPage();
		return;
	}

	LOG_DEBUG << "Custom error page: " << iter->second;

	const statusCode oldStatus = m_request.httpStatus;
	m_request.hasReturn = false;
	m_request.httpStatus = StatusOK;
	m_request.uri.path = iter->second;
	TargetResourceHandler targetResourceHandler(m_fileSystemOps);
	targetResourceHandler.execute(m_connection.m_request, m_connection.location, m_connection.serverConfig);

	if (m_request.hasReturn) {
		m_request.httpStatus = oldStatus;
		if (m_request.targetResource.empty())
			setDefaultErrorPage();
		else
			m_responseBody = m_request.targetResource;
		return;
	}

	if (m_request.httpStatus != StatusOK) {
		setDefaultErrorPage();
		return;
	}

	try {
		m_responseBody = m_fileSystemOps.getFileContents(m_request.targetResource.c_str());
	} catch (FileSystemOps::FileNotFoundException& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusNotFound;
	} catch (FileSystemOps::NoPermissionException& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusForbidden;
	} catch (const std::runtime_error& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusInternalServerError;
	}
	if (m_request.httpStatus != StatusOK)
		setDefaultErrorPage();
	else
		m_request.httpStatus = oldStatus;
}

/**
 * @brief Sets Default Error Page.
 *
 * m_responseBody is set to Default Error Page via getDefaultErrorPage().
 * The targetResource of the request is set to error.html to get the correct MIME type mapping.
 */
void ResponseBodyHandler::setDefaultErrorPage()
{
	m_responseBody = getDefaultErrorPage(m_request.httpStatus);
	m_request.targetResource = "error.html";
}

/**
 * @brief Get Default Error Page for a given status code.
 *
 * @param statusCode Status code.
 * @return std::string Default error page.
 */
std::string getDefaultErrorPage(statusCode statusCode)
{
	if (statusCode < NoStatus || statusCode > StatusNonSupportedVersion)
		statusCode = StatusInternalServerError;

	static const char* error301Page = "<html>\r\n"
									  "<head><title>301 Moved permanently</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>301 Moved permanently</h1></center>\r\n";

	static const char* error302Page = "<html>\r\n"
									  "<head><title>302 Found</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>302 Found</h1></center>\r\n";

	static const char* error308Page = "<html>\r\n"
									  "<head><title>308 Permanent redirect</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>308 Permanent redirect</h1></center>\r\n";

	static const char* error400Page = "<html>\r\n"
									  "<head><title>400 Bad request</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>400 Bad request</h1></center>\r\n";

	static const char* error403Page = "<html>\r\n"
									  "<head><title>403 Forbidden</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>403 Forbidden</h1></center>\r\n";

	static const char* error404Page = "<html>\r\n"
									  "<head><title>404 Not Found</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>404 Not Found</h1></center>\r\n";

	static const char* error405Page = "<html>\r\n"
									  "<head><title>405 Method not allowed</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>405 Method not allowed</h1></center>\r\n";

	static const char* error408Page = "<html>\r\n"
									  "<head><title>408 Request timeout</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>408 Request timeout</h1></center>\r\n";

	static const char* error413Page = "<html>\r\n"
									  "<head><title>413 Request entity too large</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>413 Request entity too large</h1></center>\r\n";

	static const char* error431Page = "<html>\r\n"
									  "<head><title>431 Request header fields too large</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>431 Request header fields too large</h1></center>\r\n";

	static const char* error500page = "<html>\r\n"
									  "<head><title>500 Internal server error</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>500 Internal server error</h1></center>\r\n";

	static const char* error501page = "<html>\r\n"
									  "<head><title>501 Method not implemented</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>501 Method not implemented</h1></center>\r\n";

	static const char* error505page = "<html>\r\n"
									  "<head><title>505 Non supported version</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>505 Non supported version</h1></center>\r\n";

	static const char* errorTail = "<hr><center>TriHard</center>\r\n"
								   "</body>\r\n"
								   "</html>\r\n";

	std::string ret;

	switch (statusCode) {
	case NoStatus:
	case StatusOK:
	case StatusCreated:
		return ("");
	case StatusMovedPermanently:
		ret = error301Page;
		break;
	case StatusFound:
		ret = error302Page;
		break;
	case StatusPermanentRedirect:
		ret = error308Page;
		break;
	case StatusBadRequest:
		ret = error400Page;
		break;
	case StatusForbidden:
		ret = error403Page;
		break;
	case StatusNotFound:
		ret = error404Page;
		break;
	case StatusRequestEntityTooLarge:
		ret = error413Page;
		break;
	case StatusMethodNotAllowed:
		ret = error405Page;
		break;
	case StatusRequestTimeout:
		ret = error408Page;
		break;
	case StatusRequestHeaderFieldsTooLarge:
		ret = error431Page;
		break;
	case StatusInternalServerError:
		ret = error500page;
		break;
	case StatusMethodNotImplemented:
		ret = error501page;
		break;
	case StatusNonSupportedVersion:
		ret = error505page;
		break;
	}

	ret += errorTail;
	return (ret);
}

/**
 * @brief Construct Allow header.
 *
 * Constructs the Allow header based on the allowed methods. Methods are appended with ", " at the end to easily join
 * them. If at the end at least one method was appended, the last ", " is removed.
 * If no methods were appended, an empty string is returned.
 * @param allowMethods Array of allowed methods.
 * @return std::string Constructed Allow header.
 */
std::string constructAllowHeader(const bool (&allowMethods)[MethodCount])
{
	std::string allowHeader;

	if (allowMethods[MethodGet])
		allowHeader.append("GET, ");
	if (allowMethods[MethodPost])
		allowHeader.append("POST, ");
	if (allowMethods[MethodDelete])
		allowHeader.append("DELETE, ");

	if (!allowHeader.empty())
		allowHeader.erase(allowHeader.size() - 2, 2);
	return allowHeader;
}

/**
 * @brief Adds special headers based on the status code.
 *
 * Location header if the status code is a redirection 3xx status.
 * Allow header if the status code is Method Not Allowed (405).
 */
void ResponseBodyHandler::addHeadersBasedOnStatus()
{
	if (isRedirectionStatus(m_request.httpStatus))
		m_responseHeaders["location"] = m_request.targetResource;

	if (m_request.httpStatus == StatusMethodNotAllowed)
		m_responseHeaders["allow"] = constructAllowHeader(m_connection.location->allowMethods);
}
