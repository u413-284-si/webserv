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

	void parseHttpRequest(const std::string& requestString, HTTPRequest& request);
    void clearRequest(HTTPRequest& request);
    void clearParser();

	// Getter functions
	int getErrorCode() const;
	Method getRequestMethod() const;

private:
	int m_errorCode;
	Method m_requestMethod;
    bool m_hasBody;
	bool m_chunked;
    std::istringstream m_requestStream;

	std::string parseMethod(const std::string& requestLine, HTTPRequest& request);
	std::string parseUri(const std::string& requestLine, HTTPRequest& request);
	void parseUriQuery(const std::string& requestLine, int& index, HTTPRequest& request);
	void parseUriFragment(const std::string& requestLine, int& index, HTTPRequest& request);
	std::string parseVersion(const std::string& requestLine, HTTPRequest& request);
	void checkHeaderName(const std::string& headerName);
	void checkContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request);
	void checkTransferEncoding(HTTPRequest& request);
	void parseChunkedBody(HTTPRequest& request);
	void parseNonChunkedBody(HTTPRequest& request);

	// Helper functions
	std::string checkForSpace(const std::string&);
	void checkForCRLF(const std::string&);
	bool isValidURIChar(uint8_t c) const;
	bool isValidHeaderFieldNameChar(uint8_t c) const;
	size_t convertHex(const std::string& chunkSize) const;
};
