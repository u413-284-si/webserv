#include "ResponseBuilder.hpp"
#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"
#include "TargetResourceHandler.hpp"

ResponseBuilder::ResponseBuilder(const ConfigFile& configFile, const FileHandler& fileHandler)
	: m_configFile(configFile)
	, m_activeServer(configFile.serverConfigs.begin())
	, m_fileHandler(fileHandler)
{
	initMIMETypes();
}

void ResponseBuilder::setActiveServer(const std::vector<ServerConfig>::const_iterator& activeServer)
{
	m_activeServer = activeServer;
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

void ResponseBuilder::appendStatusLine()
{
	m_response << "HTTP/1.1 " << m_httpResponse.status << ' ' << statusCodeToReasonPhrase(m_httpResponse.status)
			   << "\r\n";
}

void ResponseBuilder::appendHeaders(const std::size_t length, const std::string& extension)
{
	// Content-Type
	m_response << "Content-Type: " << getMIMEType(extension) << "\r\n";
	// Content-Length
	m_response << "Content-Length: " << length << "\r\n";
	// Server
	m_response << "Server: SGC-Node\r\n";
	// Date
	char date[80];
	time_t now = time(0);
	struct tm time = *gmtime(&now);
	(void)strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &time);
	m_response << "Date: " << date << "\r\n";
	// Location
	if (m_httpResponse.status == StatusMovedPermanently) {
		m_response << "Location: " << m_httpResponse.status << "\r\n";
	}
	// Delimiter
	m_response << "\r\n";
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	// m_httpResponse = request.status;
	TargetResourceHandler targetResourceHandler(m_activeServer->locations, m_fileHandler);
	m_httpResponse = targetResourceHandler.execute(request);
	m_httpResponse.method = "GET";
	ResponseBodyHandler responseBodyHandler(m_fileHandler);
	responseBodyHandler.execute(m_httpResponse);

	appendStatusLine();
	appendHeaders(m_httpResponse.body.length(), utils::getFileExtension(m_httpResponse.targetResource));
	m_response << m_httpResponse.body << "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str(); }
