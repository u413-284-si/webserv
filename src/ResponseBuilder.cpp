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
	// Server
	m_response << "Server: SGC-Node\r\n";
	// Date
	char date[80];
	time_t now = time(0);
	struct tm time = *gmtime(&now);
	(void)strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &time);
	m_response << "Date: " << date << "\r\n";
	// Location
	if (m_statusCode == StatusMovedPermanently)
	{
		m_response << "Location: " << m_location << "\r\n";
	}
	// Delimiter
	m_response << "\r\n";
}

std::vector<Location>::const_iterator ResponseBuilder::matchLocation(const std::string& path)
{
	std::size_t longestMatch = 0;
	std::vector<Location>::const_iterator locationMatch = m_activeServer->locations.end();

	for (std::vector<Location>::const_iterator it = m_activeServer->locations.begin();
		 it != m_activeServer->locations.end(); ++it)
	{
		if (path.find(it->path) == 0)
		{
			if (it->path.length() > longestMatch)
			{
				longestMatch = it->path.length();
				locationMatch = it;
			}
		}
	}
	return locationMatch;
}

void ResponseBuilder::locateTargetResource(const std::string& path)
{
	// Check which location block matches the path
	std::vector<Location>::const_iterator locationMatch = matchLocation(path);

	// No location found > do we also set a default location to not make extra check?
	if (locationMatch == m_activeServer->locations.end())
	{
		m_statusCode = StatusNotFound;
		return;
	}
	m_targetResource = locationMatch->root + path;
	if (FileHandler::isDirectory(m_targetResource))
	{
		if (m_targetResource.at(m_targetResource.length() - 1) != '/')
		{
			m_location = path + "/";
			m_statusCode = StatusMovedPermanently;
			return;
		}
		m_targetResource += locationMatch->index;
		if (!FileHandler::isExistingFile(m_targetResource))
		{
			m_statusCode = StatusForbidden;
			return;
		}
	}

}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	locateTargetResource(request.uri.path);
	if (m_statusCode != StatusOK)
	{
		appendStatusLine();
		appendHeaders(0, "txt");
		return;
	}
	const std::string fileContent = FileHandler::getFileContents(m_targetResource.c_str());
	appendStatusLine();
	appendHeaders(fileContent.length(), "txt");
	m_response << fileContent << "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str(); }
