#pragma once

/* ====== LIBRARIES ====== */

#include "HTTPRequest.hpp"
#include "StatusCode.hpp"
#include "error.hpp"
#include "utilities.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

/* ====== DEFINITIONS ====== */

const int   versionPrefixLength = 5;    // HTTP/ -> 5 chars
const int   getLength = 3;              // GET -> 3 chars
const int   postLength = 4;             // POST -> 4 chars
const int   deleteLength = 6;           // DELETE -> 6 chars
const int   decimalBase = 10;

/* ====== CLASS DECLARATION ====== */

class RequestParser {
public:
	RequestParser();

	void parseHttpRequest(const std::string& requestString, HTTPRequest& request);
	static void clearRequest(HTTPRequest& request);
	void clearParser();

private:
	bool m_hasBody;
	bool m_chunked;
	std::istringstream m_requestStream;

	static std::string parseMethod(const std::string& requestLine, HTTPRequest& request);
	static std::string parseUri(const std::string& requestLine, HTTPRequest& request);
	static void parseUriQuery(const std::string& requestLine, int& index, HTTPRequest& request);
	static void parseUriFragment(const std::string& requestLine, int& index, HTTPRequest& request);
	static std::string parseVersion(const std::string& requestLine, HTTPRequest& request);
	static void checkHeaderName(const std::string& headerName, HTTPRequest& request);
	void checkContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request);
	void checkTransferEncoding(HTTPRequest& request);
	void parseChunkedBody(HTTPRequest& request);
	void parseNonChunkedBody(HTTPRequest& request);

	// Helper functions
	static std::string checkForSpace(const std::string&, HTTPRequest& request);
	static void checkForCRLF(const std::string&, HTTPRequest& request);
	static bool isNotValidURIChar(uint8_t chr);
	static bool isValidHeaderFieldNameChar(uint8_t chr);
	static size_t convertHex(const std::string& chunkSize);
};
