#pragma once

#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "AutoindexHandler.hpp"
#include "StatusCode.hpp"
#include <exception>

/**
 * @brief Class to handle the body of a HTTP response.
 *
 * The FileSystemPolicy passed in the constructor needs to outlive this class.
 * A mock of FileSystemPolicy can be used for testing.
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
