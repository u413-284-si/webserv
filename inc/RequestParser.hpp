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

/**
 * @class RequestParser
 * @brief Parses HTTP requests and handles various request-related validations and transformations. Stateless class in
 * order to handle multiple clients concurrently.
 */
class RequestParser {
public:
	static const int s_maxLabelLength = 63; /**< Maximum length for labels (the parts between dots in a domain name)  */
	static const int s_maxHostNameLength = 253; /**< Maximum length for DNS hostname */
	static const int s_maxChunkSize = 8000; /**< Maximum size for a chunk in chunked encoding */

	RequestParser();

	// custom exceptions
	struct MethodNotImplementedException : public std::runtime_error {
		explicit MethodNotImplementedException(const std::string& msg);
	};

	struct MethodNotAllowedException : public std::runtime_error {
		explicit MethodNotAllowedException(const std::string& msg);
	};

	struct HTTPVersionNotSupportedException : public std::runtime_error {
		explicit HTTPVersionNotSupportedException(const std::string& msg);
	};

	struct RequestEntityTooLargeException : public std::runtime_error {
		explicit RequestEntityTooLargeException(const std::string& msg);
	};

	void parseHeader(const std::string& headerString, HTTPRequest& request);
	static void parseChunkedBody(std::string& bodyBuffer, HTTPRequest& request);
	static void decodeMultipartFormdata(HTTPRequest& request);
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
	static std::string decodePercentEncoding(const std::string& encoded);

	// Header Parsing
	void parseHeaders(HTTPRequest& request);
	static void extractBoundary(HTTPRequest& request);

	// Checks
	static void validateHeaderName(const std::string& headerName);
	static void validateContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request);
	static void validateHostHeader(HTTPRequest& request);
	static void validateNoMultipleHostHeaders(const std::string& headerName, HTTPRequest& request);
	static void validateTransferEncoding(HTTPRequest& request);
	static void validateMethodWithBody(HTTPRequest& request);
	static void validateConnectionHeader(HTTPRequest& request);

	// Helper functions
	static std::string checkForSpace(const std::string& str);
	static void checkForCRLF(const std::string& str);
	static bool isNotValidURIChar(uint8_t chr);
	static bool isValidHeaderFieldNameChar(uint8_t chr);
	static long convertHex(const std::string& chunkSize);
	static bool isMethodAllowedToHaveBody(HTTPRequest& request);
	static bool isValidHostnameChar(char character, bool& hasAlpha);
	static bool isValidLabel(const std::string& label, bool& hasAlpha);
	static bool isValidHostname(const std::string& hostname);
	static std::string removeDotSegments(const std::string& path);
	static bool isMultipartFormdata(HTTPRequest& request);
	static size_t checkForString(const std::string& string, size_t startPos, const std::string& body);
};
