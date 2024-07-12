#include "ResponseBodyHandler.hpp"
#include "AutoindexHandler.hpp"
#include "FileSystemPolicy.hpp"
#include "StatusCode.hpp"
#include <exception>

ResponseBodyHandler::ResponseBodyHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

void ResponseBodyHandler::handleErrorBody(HTTPResponse& response)
{
	response.body = getDefaultErrorPage(response.status);
}

HTTPResponse ResponseBodyHandler::execute(HTTPResponse& response)
{
	if (response.autoindex) {
		AutoindexHandler autoindexHandler(m_fileSystemPolicy);
		response.body = autoindexHandler.execute(response.targetResource);
		response.status = StatusOK;
		response.targetResource += "autoindex.html";
		return response;
	}
	if (response.status != StatusOK) {
		handleErrorBody(response);
		return response;
	}
	if (response.method == "GET") {
		try {
			response.body = m_fileSystemPolicy.getFileContents(response.targetResource.c_str());
		} catch (std::exception& e) {
			response.status = StatusInternalServerError;
			handleErrorBody(response);
		}
	}
	return response;
}
