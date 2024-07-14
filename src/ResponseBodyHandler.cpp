#include "ResponseBodyHandler.hpp"
#include "AutoindexHandler.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "StatusCode.hpp"
#include <exception>

ResponseBodyHandler::ResponseBodyHandler(HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy)
	: m_response(response)
	, m_fileSystemPolicy(fileSystemPolicy)
{
}

void ResponseBodyHandler::handleErrorBody()
{
	m_response.body = getDefaultErrorPage(m_response.status);
}

void ResponseBodyHandler::execute()
{
	if (m_response.autoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemPolicy);
		m_response.body = autoindexHandler.execute(m_response.targetResource);
		if (m_response.body.empty()) {
			m_response.status = StatusInternalServerError;
			handleErrorBody();
			return ;
		}
		m_response.status = StatusOK;
		m_response.targetResource += "autoindex.html";
		return ;
	}
	if (m_response.status != StatusOK) {
		handleErrorBody();
		return ;
	}
	if (m_response.method == "GET") {
		try {
			m_response.body = m_fileSystemPolicy.getFileContents(m_response.targetResource.c_str());
		} catch (std::exception& e) {
			m_response.status = StatusInternalServerError;
			handleErrorBody();
		}
	}
}
