#pragma once

#include "RequestParser.hpp"

class ResponseBuilder {
public:
	void buildResponse(const HTTPRequest& request);
	const std::string& getResponse() const;

private:
	static std::string createStatusLine(int code);
	void appendHeaders(std::size_t length);
	static std::string createHeaderContentType();
	static std::string createHeaderContentLength(std::size_t length);

	std::string m_response;
};
