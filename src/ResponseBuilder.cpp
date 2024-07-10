#include "ResponseBuilder.hpp"
#include "StatusCode.hpp"
#include "TargetResourceHandler.hpp"

ResponseBuilder::ResponseBuilder(const ConfigFile& configFile, const FileHandler& fileHandler)
	: m_statusCode(StatusOK)
	, m_configFile(configFile)
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
	if (m_mimeTypes.find(extension) != m_mimeTypes.end()) {
		return m_mimeTypes.at(extension);
	}
	return m_mimeTypes.at("default");
}

void ResponseBuilder::appendStatusLine()
{
	m_response << "HTTP/1.1 " << m_statusCode << ' ' << statusCodeToString(m_statusCode) << "\r\n";
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
	if (m_statusCode == StatusMovedPermanently) {
		m_response << "Location: " << m_location << "\r\n";
	}
	// Delimiter
	m_response << "\r\n";
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	TargetResourceHandler targetResourceHandler(m_activeServer->locations, m_fileHandler);
	m_httpResponse = targetResourceHandler.execute(request);
	if (m_httpResponse.status != StatusOK) {
		m_statusCode = m_httpResponse.status;
		appendStatusLine();
		appendHeaders(0, "txt");
		return;
	}
	const std::string fileContent = m_fileHandler.getFileContents(m_httpResponse.targetResource.c_str());
	appendStatusLine();
	appendHeaders(fileContent.length(), "txt");
	m_response << fileContent << "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str(); }
