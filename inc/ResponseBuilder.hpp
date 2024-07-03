#pragma once

#include "RequestParser.hpp"
#include <string>

class ResponseBuilder {
public:
	ResponseBuilder();

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
	void appendStatusLine(int code);
	void appendHeaders(std::size_t length, const std::string& extension);
	std::string getMIMEType(const std::string& extension);
	void initMIMETypes();

	std::stringstream m_response;
	std::map<std::string, std::string> m_mimeTypes;
	unsigned short m_statusCode;
};
