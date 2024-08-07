#pragma once

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "Log.hpp"
#include "RequestParser.hpp"
#include "ResponseBodyHandler.hpp"
#include "TargetResourceHandler.hpp"
#include <string>

/**
 * @brief Class to build a HTTP response.
 *
 * This class is responsible to build a HTTP response based on the request received.
 * The ConfigFile is used to get the configuration of the server.
 * It uses the classes TargetResourceHandler to handle the target resource and
 * ResponseBodyHandler for the body of the response.
 * The FileSystemPolicy passed in the constructor needs to outlive this class. It is passed to subclasses.
 * A mock of FileSystemPolicy can be used for testing.
 */
class ResponseBuilder {
public:
	explicit ResponseBuilder(const ConfigFile& configFile, const FileSystemPolicy& fileSystemPolicy);

	void buildResponse(const HTTPRequest& request);
	std::string getResponse() const;
	void setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer);

private:
	void appendStatusLine(const HTTPResponse& response);
	void appendHeaders(const HTTPResponse& response);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();
	static HTTPResponse initHTTPResponse(const HTTPRequest& request);
	void resetStream();

	std::map<std::string, std::string> m_mimeTypes;
	const ConfigFile& m_configFile;
	std::vector<ServerConfig>::const_iterator m_activeServer;
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_responseStream;
	bool m_isFirstTime;
};
