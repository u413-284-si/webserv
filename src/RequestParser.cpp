#include "RequestParser.hpp"
#include "Log.hpp"
#include <cstddef>

/* ====== HELPER FUNCTIONS ====== */

/**
 * @brief Checks if a given string starts with a single space followed by a non-space character.
 *
 * This function checks if the provided string `str` starts with exactly one space character followed
 * by a non-space character. If this condition is met, it returns a substring of `str` starting from
 * the second character. If the condition is not met, it sets the error code to StatusBadRequest and throws a
 * `std::runtime_error` with an appropriate error message.
 *
 * @param str The input string to be checked.
 * @param request The HTTP request object to be filled.
 * @return A substring of `str` starting from the second character if the input string starts with
 *         a single space followed by a non-space character.
 * @throws std::runtime_error If the input string does not start with a single space followed by a
 *         non-space character, an error is thrown and the error code is set to StatusBadRequest.
 */
std::string RequestParser::checkForSpace(const std::string& str, HTTPRequest& request)
{
	if (str.length() > 1 && str[0] == ' ' && str[1] != ' ')
		return (str.substr(1));
	request.httpStatus = StatusBadRequest;
	throw std::runtime_error(ERR_MISS_SINGLE_SPACE);
}

/**
 * @brief Checks if a given string starts with CRLF.
 *
 * Only checks for the carriage return \r, as the linefeed \\n is discarded
 * by std::getline
 * @param str	Input string to be checked.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If the input string does not contain a single
 *         carriage return, an error is thrown and the error code is set to StatusBadRequest.
 */
void RequestParser::checkForCRLF(const std::string& str, HTTPRequest& request)
{
	if (str.length() != 1 || str[0] != '\r') {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_MISS_CRLF);
	}
}

/**
 * @brief Checks if a given character is a valid URI character.
 *
 * This function determines if the provided character `c` is a valid character in a URI.
 * It checks against both unreserved and reserved characters as per the URI specification.
 *
 * The unreserved characters include:
 * - Alphanumeric characters (letters and digits)
 * - Hyphen (`-`)
 * - Period (`.`)
 * - Underscore (`_`)
 * - Tilde (`~`)
 *
 * The reserved characters include:
 * - Colon (`:`)
 * - Slash (`/`)
 * - Question mark (`?`)
 * - Hash (`#`)
 * - Square brackets (`[`, `]`)
 * - At symbol (`@`)
 * - Exclamation mark (`!`)
 * - Dollar sign (`$`)
 * - Ampersand (`&`)
 * - Apostrophe (`'`)
 * - Parentheses (`(`, `)`)
 * - Asterisk (`*`)
 * - Plus sign (`+`)
 * - Comma (`,`),
 * - Semicolon (`;`)
 * - Equals sign (`=`)
 *
 * @param chr The character to be checked.
 * @return `true` if the character is not a valid URI character, `false` if valid.
 */
bool RequestParser::isNotValidURIChar(uint8_t chr)
{
	// Check for unreserved chars
	if ((std::isalnum(chr) != 0) || chr == '-' || chr == '.' || chr == '_' || chr == '~')
		return false;

	// Check for reserved characters
	switch (chr) {
	case ':':
	case '/':
	case '?':
	case '#':
	case '[':
	case ']':
	case '@':
	case '!':
	case '$':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case ';':
	case '=':
		return false;
	default:
		return true;
	}
}

/**
 * @brief Checks if a given character is a valid HTTP header field name character.
 *
 * This function determines if the provided character `c` is valid for use in an HTTP header field name.
 * According to the HTTP/1.1 specification, valid characters for header field names include alphanumeric
 * characters (letters and digits) and the following special characters:
 *
 * - Exclamation mark (`!`)
 * - Hash (`#`)
 * - Dollar sign (`$`)
 * - Percent (`%`)
 * - Ampersand (`&`)
 * - Apostrophe (`'`)
 * - Asterisk (`*`)
 * - Plus sign (`+`)
 * - Hyphen (`-`)
 * - Period (`.`)
 * - Caret (`^`)
 * - Underscore (`_`)
 * - Grave accent (`\``)
 * - Vertical bar (`|`)
 * - Tilde (`~`)
 *
 * @param chr The character to be checked.
 * @return `true` if the character is valid for an HTTP header field name, `false` otherwise.
 */
bool RequestParser::isValidHeaderFieldNameChar(uint8_t chr)
{
	if (std::isalnum(chr) != 0)
		return true;

	switch (chr) {
	case '!':
	case '#':
	case '$':
	case '%':
	case '&':
	case '\'':
	case '*':
	case '+':
	case '-':
	case '.':
	case '^':
	case '_':
	case '`':
	case '|':
	case '~':
		return true;
	default:
		return false;
	}
}

/**
 * @brief Converts a hexadecimal string to a size_t value.
 *
 * This function takes a string representing a hexadecimal number,
 * validates it, and converts it to a size_t value.
 *
 * @param chunkSize The string containing the hexadecimal number.
 * @return The converted size_t value.
 *
 * @throws std::invalid_argument if the chunkSize string is empty or contains invalid hexadecimal characters.
 * @throws std::runtime_error if the conversion from string to size_t fails.
 */
size_t RequestParser::convertHex(const std::string& chunkSize)
{
	if (chunkSize.empty())
		throw std::invalid_argument(ERR_NON_EXISTENT_CHUNKSIZE);

	for (std::string::const_iterator it = chunkSize.begin(); it != chunkSize.end(); ++it) {
		if (std::isxdigit(*it) == 0)
			throw std::invalid_argument(ERR_INVALID_HEX_CHAR);
	}

	std::istringstream iss(chunkSize);
	size_t value = 0;

	iss >> std::hex >> value;
	if (iss.fail())
		throw std::runtime_error(ERR_CONVERSION_STRING_TO_HEX);
	return value;
}

/**
 * @brief Clears the contents of the given HTTPRequest object.
 *
 * This function resets all the fields of the provided HTTPRequest object
 * to their default states. It sets the HTTP method to `MethodCount`,
 * clears the URI fragment, path, and query, sets the version to an
 * empty string, clears the headers and body, sets the error code to 0,
 * and indicates that the connection should not be closed.
 *
 * @param request The HTTPRequest object to be cleared.
 */
void RequestParser::clearRequest(HTTPRequest& request)
{
	request.method = MethodCount;
	request.uri.fragment = "";
	request.uri.path = "";
	request.uri.query = "";
	request.version = "";
	request.headers.clear();
	request.body = "";
	request.httpStatus = StatusOK;
	request.shallCloseConnection = false;
}

/**
 * @brief Clears the internal state of the RequestParser object.
 *
 * This function resets the internal state of the RequestParser by
 * setting the flags `m_hasBody` and `m_chunked` to `false`, and
 * clearing the contents of the `m_requestStream` stringstream.
 */
void RequestParser::clearParser()
{
	m_hasBody = false;
	m_chunked = false;
	m_requestStream.clear();
	m_requestStream.str("");
}

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructs a new RequestParser object.
 *
 * This constructor initializes the RequestParser object with default values.
 * The `m_hasBody` member is set to `false` and the `m_chunked` member is set to `false`.
 */
RequestParser::RequestParser()
	: m_hasBody(false)
	, m_chunked(false)
{
}

/* ====== MEMBER FUNCTIONS ====== */

/**
 * @brief Parses an HTTP request string into an HTTPRequest object.
 *
 * This function takes a raw HTTP request string and parses it into an
 * `HTTPRequest` object. The parsing process involves several steps:
 * 1. Parsing the request line (method, URI, and version).
 * 2. Parsing the headers.
 * 3. Parsing the body (if any).
 *
 * If any errors are encountered during parsing, an appropriate error code is
 * set, and a `std::runtime_error` is thrown.
 *
 * @param requestString The raw HTTP request string to be parsed.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If there is any error during parsing.
 */
void RequestParser::parseHttpRequest(const std::string& requestString, HTTPRequest& request)
{
	m_requestStream.str(requestString);

	// Step 1: Parse the request-line
	std::string requestLine;
	if (!std::getline(m_requestStream, requestLine) || requestLine.empty()) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_MISS_REQUEST_LINE);
	}
	requestLine = parseMethod(requestLine, request);
	requestLine = checkForSpace(requestLine, request);
	requestLine = parseUri(requestLine, request);
	requestLine = checkForSpace(requestLine, request);
	requestLine = parseVersion(requestLine, request);
	checkForCRLF(requestLine, request);

	LOG_DEBUG << "Parsed method: " << request.method;
	LOG_DEBUG << "Parsed URI: " << request.uri.path << request.uri.query << request.uri.fragment;
	LOG_DEBUG << "Parsed version: " << request.version;

	// Step 2: Parse headers
	std::string headerLine;
	// The end of the headers section is marked by an empty line (\r\n\r\n).
	while (!std::getline(m_requestStream, headerLine).fail() && headerLine != "\r" && !headerLine.empty()) {
		if (headerLine[0] == ' ' || headerLine[0] == '\t') {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_OBSOLETE_LINE_FOLDING);
		}
		std::string headerName;
		std::string headerValue;
		const std::size_t delimiterPos = headerLine.find_first_of(':');
		if (delimiterPos != std::string::npos) {
			headerName = headerLine.substr(0, delimiterPos);
			checkHeaderName(headerName, request);
			headerValue = headerLine.substr(delimiterPos + 1);
			;
			if (headerValue[headerValue.size() - 1] == '\r')
				headerValue.erase(headerValue.size() - 1);
			headerValue = webutils::trimLeadingWhitespaces(headerValue);
			webutils::trimTrailingWhiteSpaces(headerValue);
			checkContentLength(headerName, headerValue, request);
			request.headers[headerName] = headerValue;
			LOG_DEBUG << "Parsed header: " << headerName << " -> " << headerValue;
		}
	}
	checkTransferEncoding(request);
	if (headerLine != "\r") {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_MISS_CRLF);
	}

	// Step 3: Parse body (if any)
	if (m_hasBody) {
		if (m_chunked)
			parseChunkedBody(request);
		else
			parseNonChunkedBody(request);
	}
	LOG_DEBUG << "Parsed body: " << request.body;
}

/**
 * @brief Parses the HTTP method from the request line.
 *
 * This function extracts the HTTP method from the beginning of the provided request line.
 * It supports the "GET", "POST", and "DELETE" methods. The method is stored in `m_request.method`,
 * and a corresponding Method is set in `m_requestMethod`. If the method is not recognized,
 * an error code is set to StatusMethodNotImplemented and a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the method is to be parsed.
 * @param request The HTTP request object to be filled.
 * @return A substring of `requestLine` starting from the character after the parsed method.
 * @throws std::runtime_error If the method is not implemented, an error is thrown and the error code is set to
 * StatusMethodNotImplemented.
 */
std::string RequestParser::parseMethod(const std::string& requestLine, HTTPRequest& request)
{
	const int getLength = 3; // GET -> 3 chars
	const int postLength = 4; // POST -> 4 chars
	const int deleteLength = 6; // DELETE -> 6 chars
	int index = 0;

	if (requestLine.compare(0, getLength, "GET") == 0) {
		request.method = MethodGet;
		index = getLength;
	} else if (requestLine.compare(0, postLength, "POST") == 0) {
		request.method = MethodPost;
		index = postLength;
	} else if (requestLine.compare(0, deleteLength, "DELETE") == 0) {
		request.method = MethodDelete;
		index = deleteLength;
	} else {
		request.httpStatus = StatusMethodNotImplemented;
		throw std::runtime_error(ERR_METHOD_NOT_IMPLEMENTED);
	}
	return (requestLine.substr(index));
}

/**
 * @brief Parses the URI from the request line.
 *
 * This function extracts the URI from the provided request line.
 * It validates the URI, ensuring it starts with a slash ('/'),
 * and checks each character for validity.
 * If the URI contains a query ('?') or fragment ('#'), it delegates parsing to
 * `parseUriQuery` and `parseUriFragment` respectively.
 * If an invalid character is encountered, an error code is set, and
 * a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the URI is to be parsed.
 * @param request The HTTP request object to be filled.
 * @return A substring of `requestLine` starting from the character after the parsed URI.
 * @throws std::runtime_error If the URI is missing the initial slash,
 *         contains invalid characters, or any other URI-related errors occur.
 */
std::string RequestParser::parseUri(const std::string& requestLine, HTTPRequest& request)
{
	int index = 0;
	if (requestLine.at(index) != '/') {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_URI_MISS_SLASH);
	}

	// Check URI string for invalid chars
	const std::string::const_iterator delimiterPos = find(requestLine.begin(), requestLine.end(), ' ');
	if (std::find_if(requestLine.begin(), delimiterPos, isNotValidURIChar) != delimiterPos) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_URI_INVALID_CHAR);
	}

	request.uri.path.push_back(requestLine[index]);
	while (requestLine.at(++index) != 0) {
		if (requestLine.at(index) == ' ')
			break;
		if (requestLine.at(index) == '?')
			parseUriQuery(requestLine, index, request);
		else if (requestLine.at(index) == '#')
			parseUriFragment(requestLine, index, request);
		else
			request.uri.path.push_back(requestLine.at(index));
		// FIXME: setup max. URI length?
	}
	return (requestLine.substr(index));
}

/**
 * @brief Parses the query component of the URI from the request line.
 *
 * This function extracts the query component of the URI from the provided request line.
 * It starts parsing at the given index and continues until a space or
 * fragment ('#') character is encountered.
 * Each character in the query is validated using `isValidURIChar`.
 * If an invalid character or another '?' is encountered,
 * an error code is set, and a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the query component of the URI is to be parsed.
 * @param index The current index in the request line string where the query component starts.
 *              This index is updated as the query is parsed.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If an invalid character is encountered in the query,
 *         or if another '?' is found, an error is thrown and the error code is set to StatusBadRequest.
 */
void RequestParser::parseUriQuery(const std::string& requestLine, int& index, HTTPRequest& request)
{
	while (requestLine.at(++index) != 0) {
		if (requestLine.at(index) == ' ' || requestLine.at(index) == '#') {
			index--;
			break;
		}
		if (requestLine.at(index) == '?') {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_URI_INVALID_CHAR);
		}
		request.uri.query.push_back(requestLine.at(index));
	}
}

/**
 * @brief Parses the fragment component of the URI from the request line.
 *
 * This function extracts the fragment component of the URI from the provided request line.
 * It starts parsing at the given index and continues until a space character is encountered.
 * Each character in the fragment is validated using `isValidURIChar`.
 * If an invalid character or another '#' is encountered,
 * an error code is set, and a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the fragment component of the URI is to be parsed.
 * @param index The current index in the request line string where the fragment component starts.
 *              This index is updated as the fragment is parsed.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If an invalid character is encountered in the fragment,
 *         or if another '#' is found, an error is thrown and the error code is set to StatusBadRequest.
 */
void RequestParser::parseUriFragment(const std::string& requestLine, int& index, HTTPRequest& request)
{
	while (requestLine.at(++index) != 0) {
		if (requestLine.at(index) == ' ') {
			index--;
			break;
		}
		if (requestLine.at(index) == '#') {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_URI_INVALID_CHAR);
		}
		request.uri.fragment.push_back(requestLine.at(index));
	}
}

/**
 * @brief Parses the HTTP version from the request line.
 *
 * This function extracts and validates the HTTP version from the provided request line.
 * It checks if the version format starts with "HTTP/",
 * followed by a valid major and minor version number.
 * If any format error is detected (e.g., missing digits or incorrect delimiter),
 * an appropriate error code is set, and a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the HTTP version is to be parsed.
 * @param request The HTTP request object to be filled.
 * @return A substring of `requestLine` starting from the character after the parsed version.
 * @throws std::runtime_error If the HTTP version format is invalid,
 *         an error is thrown and the error code is set to StatusBadRequest.
 */
std::string RequestParser::parseVersion(const std::string& requestLine, HTTPRequest& request)
{
	const int versionPrefixLength = 5; // HTTP/ -> 5 chars

	if (requestLine.substr(0, versionPrefixLength) != "HTTP/") {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_INVALID_VERSION_FORMAT);
	}
	int index = versionPrefixLength;
	if (isdigit(requestLine[index]) == 0) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_INVALID_VERSION_MAJOR);
	}
	if (requestLine[index] != '1') {
		request.httpStatus = StatusNonSupportedVersion;
		throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
	}
	request.version.push_back(requestLine[index]);
	if (requestLine[++index] != '.') {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_INVALID_VERSION_DELIM);
	}
	request.version.push_back(requestLine[index]);
	if (isdigit(requestLine[++index]) == 0) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_INVALID_VERSION_MINOR);
	}
	if (requestLine[index] != '1' && requestLine[index] != '0') {
		request.httpStatus = StatusNonSupportedVersion;
		throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
	}
	request.version.push_back(requestLine[index]);
	return (requestLine.substr(++index));
}

/**
 * @brief Parses a chunked body from the provided input stream.
 *
 * This function reads and processes the chunked transfer encoding format from the input stream.
 * It reads chunks of data prefixed by their size in hexadecimal format, appends the data to the
 * request body, and handles any formatting errors.
 *
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If the chunked body format is invalid (missing CRLF or incorrect chunk size).
 *
 * Error codes:
 * - ERR_MISS_CRLF: Thrown when a line does not end with a CRLF.
 * - ERR_CHUNK_SIZE: Thrown when the chunk size does not match the specified size.
 *
 * Example usage:
 * @code
 * std::istringstream requestStream("4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n");
 * RequestParser parser;
 * parser.parseChunkedBody(requestStream);
 * @endcode
 */
void RequestParser::parseChunkedBody(HTTPRequest& request)
{
	size_t length = 0;
	std::string strChunkSize;
	std::string chunkData;
	size_t numChunkSize = 0;

	do {
		std::getline(m_requestStream, strChunkSize);
		if (strChunkSize[strChunkSize.size() - 1] == '\r')
			strChunkSize.erase(strChunkSize.size() - 1);
		else {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_MISS_CRLF);
		}
		numChunkSize = convertHex(strChunkSize);
		std::getline(m_requestStream, chunkData);
		if (chunkData[chunkData.size() - 1] == '\r')
			chunkData.erase(chunkData.size() - 1);
		else {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_MISS_CRLF);
		}
		if (chunkData.size() != numChunkSize) {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_CHUNK_SIZE);
		}
		request.body += chunkData;
		length += numChunkSize;
	} while (numChunkSize > 0);
}

/**
 * @brief Parses a non-chunked body from the provided input stream.
 *
 * This function reads the entire body from the input stream, ensuring that the
 * total length of the body matches the "Content-Length" header specified in the request.
 * It processes each line, removing trailing carriage returns and concatenates
 * the lines to form the complete body.
 *
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If there is an error converting the "Content-Length" header
 *                            to a size_t or if the body length does not match the "Content-Length" value.
 *
 * Error codes:
 * - ERR_CONVERSION_STRING_TO_SIZE_T: Thrown when the conversion of "Content-Length" header to size_t fails.
 * - ERR_CONTENT_LENGTH: Thrown when the length of the parsed body does not match the "Content-Length" value.
 *
 * Example usage:
 * @code
 * std::istringstream requestStream("This is the body of the request.\r\n");
 * RequestParser parser;
 * parser.parseNonChunkedBody(requestStream);
 * @endcode
 */
void RequestParser::parseNonChunkedBody(HTTPRequest& request)
{
	std::string body;
	long length = 0;

	while (!std::getline(m_requestStream, body).fail()) {
		if (body[body.size() - 1] == '\r') {
			body.erase(body.size() - 1);
			length += 1;
		}
		if (!m_requestStream.eof())
			body += '\n';
		length += static_cast<long>(body.size());
		request.body += body;
	}
	const long contentLength = std::strtol(request.headers.at("Content-Length").c_str(), NULL, decimalBase);
	if (contentLength != length) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_CONTENT_LENGTH);
	}
}

/**
 * @brief Checks the validity of an HTTP header field name.
 *
 * This function verifies if the provided HTTP header field name is valid
 * according to HTTP specifications.
 * It ensures that the header name does not end with whitespace and that all
 * characters in the header name are valid according to `isValidHeaderFieldNameChar`.
 *
 * @param headerName The HTTP header field name to be validated.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If the header name contains invalid characters or
 *         ends with whitespace, an error is thrown and the error code is set to StatusBadRequest.
 */
void RequestParser::checkHeaderName(const std::string& headerName, HTTPRequest& request)
{
	if (isspace(headerName[headerName.size() - 1]) != 0) {
		request.httpStatus = StatusBadRequest;
		throw std::runtime_error(ERR_HEADER_COLON_WHITESPACE);
	}
	for (size_t i = 0; i < headerName.size(); i++) {
		if (!isValidHeaderFieldNameChar(headerName[i])) {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_HEADER_NAME_INVALID_CHAR);
		}
	}
}

/**
 * @brief Checks the validity of the Content-Length header field.
 *
 * This function verifies the validity of the Content-Length header field in an HTTP request.
 * It ensures that:
 * - There is only one Content-Length header or multiple with the same value.
 * - The value of Content-Length is a valid numeric string.
 * If the header is invalid or there are multiple Content-Length headers with conflicting values,
 * an appropriate error code is set, and a `std::runtime_error` is thrown.
 *
 * @param headerName The name of the HTTP header field.
 * @param headerValue The value associated with the HTTP header field.
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If there are multiple conflicting Content-Length
 *         headers or if the Content-Length value is invalid, an error is thrown
 *         and the error code is set to StatusBadRequest.
 */
void RequestParser::checkContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request)
{
	if (headerName == "Content-Length") {
		if (headerValue.empty()) {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
		}
		if (request.headers.find("Content-Length") != request.headers.end()
			&& request.headers["Content-Length"] != headerValue) {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_MULTIPLE_CONTENT_LENGTH_VALUES);
		}

		std::vector<std::string> strValues = webutils::split(headerValue, ", ");
		std::vector<long> numValues;
		for (size_t i = 0; i < strValues.size(); i++) {
			char* endptr = NULL;
			const long contentLength = std::strtol(strValues[i].c_str(), &endptr, decimalBase);
			if ((contentLength == 0) || *endptr != '\0') {
				request.httpStatus = StatusBadRequest;
				throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
			}
			numValues.push_back(contentLength);
			if (i != 0 && contentLength != numValues[i - 1]) {
				request.httpStatus = StatusBadRequest;
				throw std::runtime_error(ERR_MULTIPLE_CONTENT_LENGTH_VALUES);
			}
		}
		m_hasBody = true;
		headerValue = strValues[0];
	}
}

/**
 * @brief Checks the validity of Transfer-Encoding header field.
 *
 * This function verifies the validity of the Transfer-Encoding header field in an HTTP request.
 * It ensures that:
 * - If the Transfer-Encoding header exists, it is not empty.
 * - The last encoding in the Transfer-Encoding list is "chunked".
 * If the header is invalid or the encoding is incorrect, an appropriate error code is set,
 * and a `std::runtime_error` is thrown.
 *
 * @param request The HTTP request object to be filled.
 * @throws std::runtime_error If the Transfer-Encoding header is missing or empty,
 *         or if the last encoding is not "chunked", an error is thrown and the error code is set to StatusBadRequest.
 */
void RequestParser::checkTransferEncoding(HTTPRequest& request)
{
	if (request.headers.find("Transfer-Encoding") != request.headers.end()) {
		if (request.headers.at("Transfer-Encoding").empty()) {
			request.httpStatus = StatusBadRequest;
			throw std::runtime_error(ERR_NON_EXISTENT_TRANSFER_ENCODING);
		}

		if (request.headers.at("Transfer-Encoding").find("chunked") != std::string::npos) {
			std::vector<std::string> encodings = webutils::split(request.headers.at("Transfer-Encoding"), ", ");
			if (encodings[encodings.size() - 1] != "chunked") {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_NON_FINAL_CHUNKED_ENCODING);
			}
			m_chunked = true;
			m_hasBody = true;
		}
	}
}

RequestParser::ParsingStatus RequestParser::getStatus() const { return m_status; }

void RequestParser::setStatus(ParsingStatus status) { m_status = status; }
