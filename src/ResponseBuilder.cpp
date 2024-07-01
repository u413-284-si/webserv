#include "ResponseBuilder.hpp"

std::string ResponseBuilder::createStatusLine(const int code)
{
	std::stringstream stream;
	stream << "HTTP/1.1 " << code << " OK"
		   << "\r\n";
	return stream.str();
}

std::string ResponseBuilder::createHeaderContentType()
{
	std::stringstream stream;
	stream << "Content-Type: "
		   << "text/html"
		   << "\r\n";
	return stream.str();
}

std::string ResponseBuilder::createHeaderContentLength(const std::size_t length)
{
	std::stringstream stream;
	stream << "Content-Length: " << length << "\r\n";
	return stream.str();
}

void ResponseBuilder::appendHeaders(const std::size_t length)
{
	m_response += createHeaderContentType();
	m_response += createHeaderContentLength(length);
	m_response += "\r\n";
}

void ResponseBuilder::buildResponse(const HTTPRequest& request)
{
	m_response += createStatusLine(200);
	appendHeaders(request.body.length());
	m_response += request.body + "\r\n";
}

const std::string& ResponseBuilder::getResponse() const { return m_response; }
