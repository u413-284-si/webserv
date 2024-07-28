#pragma once

#include "ConfigFile.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include <string>
#include "TargetResourceHandler.hpp"
#include "ResponseBodyHandler.hpp"

/**
 * @brief Class to build a HTTP response.
 *
 */
class ResponseBuilder {
public:
	explicit ResponseBuilder(const ConfigFile& configFile, const FileSystemPolicy& fileSystemPolicy = FileSystemPolicy());

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
	std::stringstream m_response;
	bool m_isFirstTime;
};
