#pragma once

/* ====== LIBRARIES ====== */

#include "HTTPRequest.hpp"
#include "utilities.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

/* ====== DEFINITIONS ====== */

enum Method {MethodGet = 0, MethodPost = 1, MethodDelete = 2, MethodCount = 3};

/* ====== CLASS DECLARATION ====== */

class RequestParser {
public:
	RequestParser();

	HTTPRequest parseHttpRequest(const std::string& request);
	void clearRequest();

	// Getter functions
	int getErrorCode() const;
	Method getRequestMethod() const;

private:
	int m_errorCode;
	Method m_requestMethod;
	HTTPRequest m_request;
    bool m_hasBody;
	bool m_chunked;

	std::string parseMethod(const std::string& requestLine);
	std::string parseUri(const std::string& requestLine);
	void parseUriQuery(const std::string& requestLine, int& index);
	void parseUriFragment(const std::string& requestLine, int& index);
	std::string parseVersion(const std::string& requestLine);
	void checkHeaderName(const std::string& headerName);
	void checkContentLength(const std::string& headerName, std::string& headerValue);
	void checkTransferEncoding();
	void parseChunkedBody(std::istringstream& requestStream);
	void parseNonChunkedBody(std::istringstream& requestStream);

	// Helper functions
	std::string checkForSpace(const std::string&);
	void checkForCRLF(const std::string&);
	bool isValidURIChar(uint8_t c) const;
	bool isValidHeaderFieldNameChar(uint8_t c) const;
	size_t convertHex(const std::string& chunkSize) const;
};
