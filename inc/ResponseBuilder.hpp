#pragma once

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Class to build a HTTP response.
 *
 * This class is responsible to build a HTTP response based on the request received.
 * It uses a stringstream m_responseHeader to convert variables to string. The stringstream is reused.
 * It uses the class ResponseBodyHandler to construct the body of the response.
 * Each time a new response is built the old one is overwritten.
 * The FileSystemPolicy passed in the constructor needs to outlive this class. It is passed to subclasses.
 * A mock of FileSystemPolicy can be used for testing.
 */
class ResponseBuilder {
public:
	explicit ResponseBuilder(const FileSystemPolicy& fileSystemPolicy);

	void buildResponse(Connection& connection);
	std::string getResponse() const;

private:
	void appendResponseHeader(const HTTPRequest& request);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();
	void resetBuilder();
	void setHeaderForStatusCode(const HTTPRequest& request, const bool (&allowedMethods)[MethodCount]);
	bool checkForExistingHeader(const std::string& headerName);

	std::map<std::string, std::string> m_mimeTypes;
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_responseHeaderStream;
	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
};

std::string constructAllowHeader(const bool (&allowedMethods)[MethodCount]);
