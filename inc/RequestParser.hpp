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
#include <cstring>
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
	static const int s_maxChunkSize = 8000; /**< Maximum size for a chunk in chunked encoding */

	RequestParser();

	void parseHeader(const std::string& headerString, HTTPRequest& request);
	static void parseChunkedBody(const std::string& bodyBuffer, HTTPRequest& request);
	void decodeMultipartFormdata(HTTPRequest& request);
	static void clearRequest(HTTPRequest& request);
	void resetRequestStream();

	const std::string& getBoundary() const;
	void setBoundary(const std::string& boundary);

private:
	std::istringstream m_requestStream;
	std::string m_boundary;

	// RequestLine Parsing
	void parseRequestLine(HTTPRequest& request);
	static std::string parseMethod(const std::string& requestLine, HTTPRequest& request);
	static std::string parseUri(const std::string& requestLine, HTTPRequest& request);
	static void parseUriQuery(const std::string& requestLine, int& index, HTTPRequest& request);
	static void parseUriFragment(const std::string& requestLine, int& index, HTTPRequest& request);
	static std::string parseVersion(const std::string& requestLine, HTTPRequest& request);
	static std::string decodePercentEncoding(const std::string& encoded, HTTPRequest& request);

	// Header Parsing
	void parseHeaders(HTTPRequest& request);
	void extractBoundary(HTTPRequest& request);

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
	static long convertHex(const std::string& chunkSize);
	static bool isMethodAllowedToHaveBody(HTTPRequest& request);
	static bool isValidHostnameChar(char character, bool& hasAlpha);
	static bool isValidLabel(const std::string& label, bool& hasAlpha);
	static bool isValidHostname(const std::string& hostname);
	static std::string removeDotSegments(const std::string& path, HTTPRequest& request);
	static bool isMultipartFormdata(HTTPRequest& request);
	static size_t checkForString(const std::string& string, size_t startPos, HTTPRequest& request);
};
