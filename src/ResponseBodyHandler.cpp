#include "ResponseBodyHandler.hpp"
#include "ConfigFile.hpp"

/**
 * @brief Construct a new ResponseBodyHandler object
 *
 * @param request The HTTP request.
 * @param responseBody Saves the response body.
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBodyHandler::ResponseBodyHandler(
	HTTPRequest& request, std::string& responseBody, const FileSystemPolicy& fileSystemPolicy)
	: m_request(request)
	, m_responseBody(responseBody)
	, m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Create the response body.
 *
 * Depending on the HTTP Request status, the body will be created.
 * If the status is not OK, an error page will be created.
 * If the status is OK, the body will be created based on the target resource.
 * If the target resource is a directory, and autoindex is on an autoindex will be created.
 * If the target resource is a file, the file contents will be read and set as the body.
 * @todo FIXME: Implement other methods than GET.
 */
void ResponseBodyHandler::execute()
{
	if (m_request.httpStatus != StatusOK) {
		handleErrorBody();
	} else if (m_request.hasAutoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemPolicy);
		m_responseBody = autoindexHandler.execute(m_request.targetResource);
		if (m_responseBody.empty()) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
			return;
		}
		m_request.httpStatus = StatusOK;
		m_request.targetResource += "autoindex.html";
	} else if (m_request.method == MethodGet) {
		try {
			m_responseBody = m_fileSystemPolicy.getFileContents(m_request.targetResource.c_str());
		} catch (std::exception& e) {
			m_request.httpStatus = StatusInternalServerError;
			handleErrorBody();
		}
	}
}

/**
 * @brief Get the error page.
 *
 * @todo FIXME: Implement custom error pages.
 */
void ResponseBodyHandler::handleErrorBody()
{
	m_responseBody = webutils::getDefaultErrorPage(m_request.httpStatus);
	// This is just to get the right extension. Could be put into its own data field of HTML struct.
	m_request.targetResource = "error.html";
}
