#pragma once

#include "RequestParser.hpp"
#include <string>

class ResponseBuilder {
public:
	ResponseBuilder();

	void buildResponse(const HTTPRequest& request);
	std::string getResponse() const;

private:
	void appendStatusLine(int code);
	void appendHeaders(std::size_t length, const std::string& extension);
	std::string getMIMEType(const std::string& extension);

	std::stringstream m_response;
	std::map<std::string, std::string> m_mimeTypes;
};
