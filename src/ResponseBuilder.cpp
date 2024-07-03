#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder(const ConfigFile& configFile)
	: m_statusCode(StatusOK)
	, m_configFile(configFile)
	, m_activeServer(configFile.serverConfigs.begin())
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
	m_response << "HTTP/1.1 " << m_statusCode << " OK"
			   << "\r\n";
}

void ResponseBuilder::appendHeaders(const std::size_t length, const std::string& extension)
{
	// Content-Type
	m_response << "Content-Type: " << getMIMEType(extension) << "\r\n";
	// Content-Length
	m_response << "Content-Length: " << length << "\r\n";
	// Delimiter
	m_response << "\r\n";
	// Server
	m_response << "Server: SGC-Node\r\n";
	// Date
	char date[80];
	time_t now = time(0);
	struct tm time = *gmtime(&now);
	(void)strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &time);
	m_response << "Date: " << date << "\r\n";
}

void ResponseBuilder::locateTargetResource(const std::string& path)
{
	m_targetResource = m_activeServer->locations[0].root + path;
	struct stat buffer = {};
	errno = 0;
	if (stat(m_targetResource.c_str(), &buffer) == -1)
		std::cerr << "error: stat: " << strerror(errno) << "\n";
	if (S_ISDIR(static_cast<unsigned int>(buffer.st_mode)))
		m_targetResource += m_activeServer->locations[0].index;
	if (access(m_targetResource.c_str(), R_OK) == -1)
		std::cout << "Target resource: " << m_targetResource << "does not exist\n";
	else
		std::cout << "Target resource: " + m_targetResource + " exists\n";
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	locateTargetResource(request.uri.path);
	appendStatusLine();
	appendHeaders(request.body.length(), "txt");
	m_response << request.body + "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str(); }
