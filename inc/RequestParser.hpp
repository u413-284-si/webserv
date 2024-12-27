#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "StatusCode.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "utilities.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class RequestParser {
public:
	static const int s_maxLabelLength = 63; /**< Maximum length for labels (the parts between dots in a domain name)  */
	static const int s_maxHostNameLength = 253; /**< Maximum length for DNS hostname */

	RequestParser();

	void parseHeader(const std::string& headerString, HTTPRequest& request);
	void parseBody(const std::string& bodyString, HTTPRequest& request);
	static void clearRequest(HTTPRequest& request);
	void resetRequestStream();

private:
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
	static void validateHeaderName(const std::string& headerName, HTTPRequest& request);
	static void validateContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request);
	static void validateHostHeader(HTTPRequest& request);
	static void validateNoMultipleHostHeaders(const std::string& headerName, HTTPRequest& request);
	static void validateTransferEncoding(HTTPRequest& request);
	static void validateMethodWithBody(HTTPRequest& request);

	// Helper functions
	static std::string checkForSpace(const std::string& str, HTTPRequest& request);
	static void checkForCRLF(const std::string& str, HTTPRequest& request);
	static bool isNotValidURIChar(uint8_t chr);
	static bool isValidHeaderFieldNameChar(uint8_t chr);
	static size_t convertHex(const std::string& chunkSize);
	static bool isMethodAllowedToHaveBody(HTTPRequest& request);
	static bool isValidHostnameChar(char character, bool& hasAlpha);
	static bool isValidLabel(const std::string& label, bool& hasAlpha);
	static bool isValidHostname(const std::string& hostname);
	static std::string removeDotSegments(const std::string& path, HTTPRequest& request);
};

