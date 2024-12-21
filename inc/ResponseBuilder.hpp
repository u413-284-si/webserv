#pragma once

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "ResponseBodyHandler.hpp"

#include <string>

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
	void appendStatusLine(const HTTPRequest& request);
	void appendHeaders(const HTTPRequest& request);
	void appendHeadersCGI(const HTTPRequest& request);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();
	void parseResponseBody(HTTPRequest& request);
	void resetBuilder();

	std::map<std::string, std::string> m_mimeTypes;
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_responseHeader;
	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
};
