#include "ResponseBodyHandler.hpp"
#include "ConfigFile.hpp"

/**
 * @brief Construct a new ResponseBodyHandler object
 *
 * @param response HTTP response.
 * @param fileSystemPolicy File system policy. Can be mocked if needed.
 */
ResponseBodyHandler::ResponseBodyHandler(HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy)
	: m_response(response)
	, m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Create the response body.
 *
 * Depending on the HTTP Response object status, the body will be created.
 * If the status is not OK, an error page will be created.
 * If the status is OK, the body will be created based on the target resource.
 * If the target resource is a directory, and autoindex is on an autoindex will be created.
 * If the target resource is a file, the file contents will be read and set as the body.
 * @todo FIXME: Implement other methods than GET.
 */
void ResponseBodyHandler::execute(Connection& connection)
{
	if (m_response.status != StatusOK) {
		handleErrorBody();
	}
	else if (m_response.isCGI) {
		CGIHandler cgiHandler(m_response.location->cgiPath, m_response.location->cgiExt);
        cgiHandler.init(const int &clientSocket, HTTPRequest &request, const Location &location, const unsigned short &serverPort);
		cgiHandler.execute(m_response);
        connection.m_pipeToCGIWriteEnd = cgiHandler.getPipeInWriteEnd();
        connection.m_pipeFromCGIReadEnd = cgiHandler.getPipeOutReadEnd();
        connection.m_cgiPid = cgiHandler.getCGIPid();
	}
	else if (m_response.isAutoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemPolicy);
		m_response.body = autoindexHandler.execute(m_response.targetResource);
		if (m_response.body.empty()) {
			m_response.status = StatusInternalServerError;
			handleErrorBody();
			return ;
		}
		m_response.status = StatusOK;
		m_response.targetResource += "autoindex.html";
	}
	else if (m_response.method == MethodGet) {
		try {
			m_response.body = m_fileSystemPolicy.getFileContents(m_response.targetResource.c_str());
		} catch (std::exception& e) {
			m_response.status = StatusInternalServerError;
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
	m_response.body = webutils::getDefaultErrorPage(m_response.status);
	// This is just to get the right extension. Could be put into its own data field of HTML struct.
	m_response.targetResource = "error.html";
}
