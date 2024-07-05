#pragma once

#include "ConfigFile.hpp"
#include "RequestParser.hpp"
#include <string>

class ResponseBuilder {
public:
	explicit ResponseBuilder(const ConfigFile& configFile);

	void buildResponse(const HTTPRequest& request);
	std::string getResponse() const;

	enum statusCode {
		StatusOK = 200,
		StatusBadRequest = 400,
		StatusNotFound = 404,
		StatusMethodNotAllowed = 405,
		StatusInternalServerError = 500
	};

private:
	void appendStatusLine();
	void appendHeaders(std::size_t length, const std::string& extension);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();
	void locateTargetResource(const std::string& path);
	std::vector<Location>::const_iterator matchLocation(const std::string& path);

	void setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer);

	std::stringstream m_response;
	std::map<std::string, std::string> m_mimeTypes;
	unsigned short m_statusCode;
	const ConfigFile& m_configFile;
	std::vector<ServerConfig>::const_iterator m_activeServer;
	std::string m_targetResource;
};
