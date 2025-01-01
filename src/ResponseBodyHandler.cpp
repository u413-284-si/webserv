#include "ResponseBodyHandler.hpp"

/**
 * @brief Construct a new ResponseBodyHandler object
 *
 * @param connection The Connection for which the response body is handled.
 * @param responseBody Saves the response body.
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBodyHandler::ResponseBodyHandler(
	Connection& connection, std::string& responseBody, const FileSystemPolicy& fileSystemPolicy)
	: m_connection(connection)
	, m_request(connection.m_request)
	, m_responseBody(responseBody)
	, m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Create the response body.
 *
 * Depending on the HTTP Request status, the body will be created:
 * - If the request had a location with Return directive additional checks are made.
 *  - If there is no Return message and status code is not an error code no body is sent.
 *  - If there is a Return message and it is not a redirection status, the target resource will be set as
 * the body.
 * - If the status is an error status an error page will be created.
 * - If the request hasAutoindex (which indicates target resource is directory) an autoindex will be created.
 * - In case of GET request (which indicates target resource is a file), the file contents will be read and set as the
 * body.
 * - In case of a POST request, the request body will be written to the target resource.
 * - In case of a DELETE request, the target resource will be deleted.
 */
void ResponseBodyHandler::execute()
{
	if (m_request.hasReturn) {
		const bool isEmpty = m_request.targetResource.empty();
		if (isEmpty && m_request.httpStatus < StatusMovedPermanently)
			return;
		if (!isEmpty && !webutils::isRedirectionStatus(m_request.httpStatus)) {
			m_responseBody = m_request.targetResource;
			return;
		}
	}

	if (m_request.httpStatus >= StatusMovedPermanently) {
		handleErrorBody();
		return;
	}
	if (m_request.hasCGI) {
		m_responseBody = m_request.body;
		if (m_responseBody.find("Content-Type: ") == std::string::npos) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
		}
		return;
	}
	if (m_request.hasAutoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemPolicy);
		m_responseBody = autoindexHandler.execute(m_request.targetResource);
		if (m_responseBody.empty()) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
			return;
		}
		m_request.httpStatus = StatusOK;
		m_request.targetResource += "autoindex.html";
		return;
	}
	if (m_request.method == MethodGet) {
		try {
			m_responseBody = m_fileSystemPolicy.getFileContents(m_request.targetResource.c_str());
		} catch (FileSystemPolicy::FileNotFoundException& e) {
			LOG_ERROR << e.what();
			m_request.httpStatus = StatusNotFound;
		} catch (FileSystemPolicy::NoPermissionException& e) {
			LOG_ERROR << e.what();
			m_request.httpStatus = StatusForbidden;
		} catch (const std::runtime_error& e) {
			LOG_ERROR << e.what();
			m_request.httpStatus = StatusInternalServerError;
		}
		if (m_request.httpStatus != StatusOK)
			handleErrorBody();
		return;
	}

	if (m_request.method == MethodPost) {
		FileWriteHandler fileWriteHandler(m_fileSystemPolicy);
		m_responseBody = fileWriteHandler.execute(m_request.targetResource, m_request.body);
		if (m_responseBody.find("created") != std::string::npos) {
			m_request.httpStatus = StatusCreated;
			m_request.headers["location"] = m_request.uri.path;
		}
		if (m_responseBody.empty()) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
		}
		m_request.targetResource = "posted.json";
		return;
	}
	if (m_request.method == MethodDelete) {
		DeleteHandler deleteHandler(m_fileSystemPolicy);
		m_responseBody = deleteHandler.execute(m_request.targetResource, m_request.httpStatus);
		if (m_responseBody.empty())
			handleErrorBody();
		m_request.targetResource = "deleted.json";
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
	TargetResourceHandler targetResourceHandler(m_fileSystemPolicy);
	targetResourceHandler.execute(m_connection);

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
		m_responseBody = m_fileSystemPolicy.getFileContents(m_request.targetResource.c_str());
	} catch (FileSystemPolicy::FileNotFoundException& e) {
		LOG_ERROR << e.what();
		m_request.httpStatus = StatusNotFound;
	} catch (FileSystemPolicy::NoPermissionException& e) {
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
