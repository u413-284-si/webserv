#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder()
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
	if (m_mimeTypes.find(extension) != m_mimeTypes.end())
	{
		return m_mimeTypes.at(extension);
	}
	return m_mimeTypes.at("default");
}

void ResponseBuilder::appendStatusLine(const int code)
{
	m_response << "HTTP/1.1 " << code << " OK"
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
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	appendStatusLine(200);
	appendHeaders(request.body.length(), "txt");
	m_response << request.body + "\r\n";
}

std::string ResponseBuilder::getResponse() const { return m_response.str(); }
