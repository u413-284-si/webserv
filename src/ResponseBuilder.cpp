#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder(const ConfigFile& configFile, const FileSystemPolicy& fileSystemPolicy)
	: m_configFile(configFile)
	, m_activeServer(configFile.serverConfigs.begin())
	, m_fileSystemPolicy(fileSystemPolicy)
	, m_isFirstTime(true)
{
	initMIMETypes();
}

void ResponseBuilder::setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer)
{
	m_activeServer = activeServer;
}

void ResponseBuilder::resetStream()
{
	if (!m_isFirstTime)
		m_response.seekp(std::ios::beg);

	m_isFirstTime = false;
}

void ResponseBuilder::initMIMETypes()
{
	m_mimeTypes["html"] = "text/html";
	m_mimeTypes["htm"] = "text/html";
	m_mimeTypes["jpg"] = "image/jpeg";
	m_mimeTypes["jpeg"] = "image/jpeg";
	m_mimeTypes["png"] = "image/png";
	m_mimeTypes["gif"] = "image/gif";
	m_mimeTypes["css"] = "text/css";
	m_mimeTypes["js"] = "application/javascript";
	m_mimeTypes["pdf"] = "application/pdf";
	m_mimeTypes["txt"] = "text/plain";
	m_mimeTypes["default"] = "application/octet-stream";
}

std::string ResponseBuilder::getMIMEType(const std::string& extension)
{
	std::map<std::basic_string<char>, std::basic_string<char> >::const_iterator iter = m_mimeTypes.find(extension);
	if (iter != m_mimeTypes.end()) {
		return iter->second;
	}
	return m_mimeTypes.at("default");
}

void ResponseBuilder::appendStatusLine(const HTTPResponse& response)
{
	m_response << "HTTP/1.1 " << response.status << ' ' << statusCodeToReasonPhrase(response.status) << "\r\n";
}

void ResponseBuilder::appendHeaders(const HTTPResponse& response)
{
	// Content-Type
	m_response << "Content-Type: " << getMIMEType(utils::getFileExtension(response.targetResource)) << "\r\n";
	// Content-Length
	m_response << "Content-Length: " << response.body.length() << "\r\n";
	// Server
	m_response << "Server: TriHard\r\n";
	// Date
	m_response << "Date: " << utils::getGMTString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	// Location
	if (response.status == StatusMovedPermanently) {
		m_response << "Location: " << response.targetResource << "\r\n";
	}
	// Delimiter
	m_response << "\r\n";
}

HTTPResponse ResponseBuilder::initHTTPResponse(const HTTPRequest& request)
{
	HTTPResponse response;

	response.status = StatusOK;
	response.method = request.method;
	response.autoindex = false;

	return response;
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	resetStream();
	
	HTTPResponse response = initHTTPResponse(request);

	TargetResourceHandler targetResourceHandler(m_activeServer->locations, request, response, m_fileSystemPolicy);
	targetResourceHandler.execute();

	ResponseBodyHandler responseBodyHandler(response, m_fileSystemPolicy);
	responseBodyHandler.execute();

	appendStatusLine(response);
	appendHeaders(response);
	m_response << response.body << "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str().c_str(); }
