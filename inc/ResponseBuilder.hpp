#pragma once

#include "ConfigFile.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "FileHandler.hpp"
#include "StatusCode.hpp"
#include <string>
#include "TargetResourceHandler.hpp"
#include "utils.hpp"

class ResponseBuilder {
public:
	explicit ResponseBuilder(const ConfigFile& configFile, const FileHandler& fileHandler);

	void buildResponse(const HTTPRequest& request);
	std::string getResponse() const;

private:
	void appendStatusLine();
	void appendHeaders(std::size_t length, const std::string& extension);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();

	void setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer);

	std::stringstream m_response;
	std::map<std::string, std::string> m_mimeTypes;
	const ConfigFile& m_configFile;
	std::vector<ServerConfig>::const_iterator m_activeServer;
	const FileHandler& m_fileHandler;
	HTTPResponse m_httpResponse;
};
