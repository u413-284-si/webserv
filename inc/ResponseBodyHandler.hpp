#pragma once

#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "AutoindexHandler.hpp"
#include "StatusCode.hpp"
#include <exception>

/**
 * @brief Class to handle the body of a HTTP response.
 *
 */
class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy);
	void execute();

private:
	void handleErrorBody();

	HTTPResponse& m_response;
	const FileSystemPolicy& m_fileSystemPolicy;
};
