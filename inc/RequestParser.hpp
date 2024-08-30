#pragma once

/* ====== LIBRARIES ====== */

#include "HTTPRequest.hpp"
#include "StatusCode.hpp"
#include "error.hpp"
#include "utilities.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

/* ====== DEFINITIONS ====== */

const int decimalBase = 10;

/* ====== CLASS DECLARATION ====== */

class RequestParser {
public:
	RequestParser();

	void parseHeader(const std::string& headerString, HTTPRequest& request);
    void parseBody(const std::string& bodyString, HTTPRequest& request);
	static void clearRequest(HTTPRequest& request);
	void clearParser();

	bool hasBody() const;
	bool isChunked() const;

private:
	bool m_hasBody;
	bool m_isChunked;
	std::istringstream m_requestStream;

	// RequestLine Parsing
	void parseRequestLine(HTTPRequest& request);
	static std::string parseMethod(const std::string& requestLine, HTTPRequest& request);
	static std::string parseUri(const std::string& requestLine, HTTPRequest& request);
	static void parseUriQuery(const std::string& requestLine, int& index, HTTPRequest& request);
	static void parseUriFragment(const std::string& requestLine, int& index, HTTPRequest& request);
	static std::string parseVersion(const std::string& requestLine, HTTPRequest& request);

	// Header Parsing
	void parseHeaders(HTTPRequest& request);

	// Body Parsing
	void parseChunkedBody(HTTPRequest& request);
	void parseNonChunkedBody(HTTPRequest& request);

	// Checks
	static void checkHeaderName(const std::string& headerName, HTTPRequest& request);
	void checkContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request);
	void checkTransferEncoding(HTTPRequest& request);
	static bool checkForCompleteBody(const std::string& bodyString, HTTPRequest& request);
	static bool checkIfMethodCanHaveBody(HTTPRequest& request);

	// Helper functions
	static std::string checkForSpace(const std::string& str, HTTPRequest& request);
	static void checkForCRLF(const std::string& str, HTTPRequest& request);
	static bool isNotValidURIChar(uint8_t chr);
	static bool isValidHeaderFieldNameChar(uint8_t chr);
	static size_t convertHex(const std::string& chunkSize);
	void resetRequestStream();
};
