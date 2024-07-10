#include "ResponseBodyHandler.hpp"
#include "FileHandler.hpp"
#include "StatusCode.hpp"
#include <exception>

ResponseBodyHandler::ResponseBodyHandler(const FileHandler& fileHandler)
	: m_fileHandler(fileHandler)
{
}

void ResponseBodyHandler::handleErrorBody(HTTPResponse& response)
{
	response.body = getDefaultErrorPage(response.status);
}

HTTPResponse ResponseBodyHandler::execute(HTTPResponse& response)
{
	if (response.status != StatusOK) {
		handleErrorBody(response);
		return response;
	}
	if (response.method == "GET") {
		try {
			response.body = m_fileHandler.getFileContents(response.targetResource.c_str());
		} catch (std::exception& e) {
			response.status = StatusInternalServerError;
			handleErrorBody(response);
		}
	}
	return response;
}
