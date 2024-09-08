#pragma once

#include "CGIHandler.hpp"
#include "Connection.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPRequest.hpp"
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
	explicit ResponseBodyHandler(HTTPRequest& request, std::string& responseBody, const FileSystemPolicy& fileSystemPolicy);
	void execute();

private:
	void handleErrorBody();

	HTTPRequest& m_request;
	std::string& m_responseBody;
	const FileSystemPolicy& m_fileSystemPolicy;
};
