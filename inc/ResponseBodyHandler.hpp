#pragma once

#include "AutoindexHandler.hpp"
#include "Connection.hpp"
#include "DeleteHandler.hpp"
#include "FileSystemOps.hpp"
#include "FileWriteHandler.hpp"
#include "Log.hpp"
#include "Method.hpp"
#include "TargetResourceHandler.hpp"

#include "cassert"

/**
 * @brief Class to handle the body of a HTTP response.
 *
 * The FileSystemOps passed in the constructor needs to outlive this class.
 * A mock of FileSystemOps can be used for testing.
 */
class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(Connection& connection, std::string& responseBody, const FileSystemOps& fileSystemOps);
	void execute();

private:
	void handleErrorBody();
	void setDefaultErrorPage();

	Connection& m_connection;
	HTTPRequest& m_request;
	std::string& m_responseBody;
	const FileSystemOps& m_fileSystemOps;
};

std::string getDefaultErrorPage(statusCode status);
