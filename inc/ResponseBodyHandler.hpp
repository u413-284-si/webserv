#pragma once

#include "AutoindexHandler.hpp"
#include "Connection.hpp"
#include "DeleteHandler.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include "Method.hpp"
#include "FileWriteHandler.hpp"
#include "TargetResourceHandler.hpp"

#include "cassert"

/**
 * @brief Class to handle the body of a HTTP response.
 *
 * The FileSystemPolicy passed in the constructor needs to outlive this class.
 * A mock of FileSystemPolicy can be used for testing.
 */
class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(
		Connection& connection, std::string& responseBody, const FileSystemPolicy& fileSystemPolicy);
	void execute();

private:
	void handleErrorBody();
	void setDefaultErrorPage();

	Connection& m_connection;
	HTTPRequest& m_request;
	std::string& m_responseBody;
	const FileSystemPolicy& m_fileSystemPolicy;
};

std::string getDefaultErrorPage(statusCode status);
