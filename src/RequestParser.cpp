#include "RequestParser.hpp"
#include "error.hpp"
#include <stdexcept>

/* ====== HELPER FUNCTIONS ====== */

/**
 * @brief Checks if a given string starts with a single space followed by a non-space character.
 *
 * This function checks if the provided string `str` starts with exactly one space character followed
 * by a non-space character. If this condition is met, it returns a substring of `str` starting from
 * the second character. If the condition is not met, it sets the error code to 400 and throws a
 * `std::runtime_error` with an appropriate error message.
 *
 * @param str The input string to be checked.
 * @return A substring of `str` starting from the second character if the input string starts with
 *         a single space followed by a non-space character.
 * @throws std::runtime_error If the input string does not start with a single space followed by a
 *         non-space character, an error is thrown and the error code is set to 400.
 */
std::string RequestParser::checkForSpace(const std::string& str)
{
	if (str.length() > 1 && str[0] == ' ' && str[1] != ' ')
		return (str.substr(1));
	m_errorCode = 400;
	throw std::runtime_error(ERR_MISS_SINGLE_SPACE);
}

/**
 * @brief Checks if a given string starts with CRLF.
 *
 * Only checks for the carriage return \r, as the linefeed \\n is discarded
 * by std::getline
 * @param str	Input string to be checked.
 * @throws std::runtime_error If the input string does not contain a single
 *         carriage return, an error is thrown and the error code is set to 400.
 */
void RequestParser::checkForCRLF(const std::string& str)
{
	if (str.length() != 1 || str[0] != '\r') {
		m_errorCode = 400;
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
 * @param c The character to be checked.
 * @return `true` if the character is a valid URI character, `false` otherwise.
 */
bool RequestParser::isValidURIChar(uint8_t c) const
{
	// Check for unreserved chars
	if (std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~')
		return true;

	// Check for reserved characters
	switch (c) {
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
		return true;
	default:
		return false;
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
 * @param c The character to be checked.
 * @return `true` if the character is valid for an HTTP header field name, `false` otherwise.
 */
bool RequestParser::isValidHeaderFieldNameChar(uint8_t c) const
{
	if (std::isalnum(c))
		return true;

	switch (c) {
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
size_t RequestParser::convertHex(const std::string& chunkSize) const
{
	if (chunkSize.empty())
		throw std::invalid_argument(ERR_NON_EXISTENT_CHUNKSIZE);

	for (std::string::const_iterator it = chunkSize.begin(); it != chunkSize.end(); ++it) {
		if (!std::isxdigit(*it))
			throw std::invalid_argument(ERR_INVALID_HEX_CHAR);
	}

	std::istringstream iss(chunkSize);
	size_t value = 0;

	iss >> std::hex >> value;
	if (iss.fail())
		throw std::runtime_error(ERR_CONVERSION_STRING_TO_HEX);
	return value;
}

void RequestParser::clearRequest()
{
	m_request.body = "";
	m_request.method = "";
	m_request.uri.fragment = "";
	m_request.uri.path = "";
	m_request.uri.query = "";
	m_request.version = "";
	m_request.headers.clear();
}

void RequestParser::clearParser()
{
    m_errorCode = 0;
    m_requestMethod = MethodCount;
    m_hasBody = false;
    m_chunked = false;
    m_requestStream.clear();
    m_requestStream.str("");
    clearRequest();
}

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

RequestParser::RequestParser()
	: m_errorCode(0)
	, m_requestMethod(MethodCount)
{
	m_hasBody = false;
	m_chunked = false;
}

/* ====== GETTER FUNCTIONS ====== */

/**
 * @brief Retrieves the current error code.
 *
 * This function returns the current error code stored in the `RequestParser` object.
 * The error code indicates the type of error that occurred during the parsing process.
 *
 * @return The current error code as an integer.
 */
int RequestParser::getErrorCode() const { return m_errorCode; }

/**
 * @brief Retrieves the bit flag representing the HTTP request method.
 *
 * This function returns the Method stored in the `RequestParser` object that represents the HTTP request method.
 * The Method is set during the parsing process based on the method type (e.g., GET, POST, DELETE).
 *
 * @return The Method corresponding to the HTTP request method.
 */
Method RequestParser::getRequestMethod() const { return m_requestMethod; }

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
 * @param request The raw HTTP request string to be parsed.
 * @return An `HTTPRequest` object representing the parsed HTTP request.
 * @throws std::runtime_error If there is any error during parsing.
 */
HTTPRequest RequestParser::parseHttpRequest(const std::string& request)
{
	m_requestStream.str(request);

	// Step 1: Parse the request-line
	std::string requestLine;
	if (!std::getline(m_requestStream, requestLine) || requestLine.empty()) {
		m_errorCode = 400;
		throw std::runtime_error(ERR_MISS_REQUEST_LINE);
	}
	requestLine = parseMethod(requestLine);
	requestLine = checkForSpace(requestLine);
	requestLine = parseUri(requestLine);
	requestLine = checkForSpace(requestLine);
	requestLine = parseVersion(requestLine);
	checkForCRLF(requestLine);

	// Step 2: Parse headers
	std::string headerLine;
	// The end of the headers section is marked by an empty line (\r\n\r\n).
	while (std::getline(m_requestStream, headerLine) && headerLine != "\r" && !headerLine.empty()) {
		if (headerLine[0] == ' ' || headerLine[0] == '\t') {
			m_errorCode = 400;
			throw std::runtime_error(ERR_OBSOLETE_LINE_FOLDING);
		}
		std::istringstream headerStream(headerLine);
		std::string headerName;
		std::string headerValue;
		if (std::getline(headerStream, headerName, ':')) {
			checkHeaderName(headerName);
			std::getline(headerStream >> std::ws, headerValue);
			if (headerValue[headerValue.size() - 1] == '\r')
				headerValue.erase(headerValue.size() - 1);
			headerValue = trimTrailingWhiteSpaces(headerValue);
			checkContentLength(headerName, headerValue);
			m_request.headers[headerName] = headerValue;
		}
	}
	checkTransferEncoding();
	if (headerLine != "\r") {
		m_errorCode = 400;
		throw std::runtime_error(ERR_MISS_CRLF);
	}

	// Step 3: Parse body (if any)
	if (m_hasBody) {
		if (m_chunked)
			parseChunkedBody();
		else
			parseNonChunkedBody();
	}
	return m_request;
}

/**
 * @brief Parses the HTTP method from the request line.
 *
 * This function extracts the HTTP method from the beginning of the provided request line.
 * It supports the "GET", "POST", and "DELETE" methods. The method is stored in `m_request.method`,
 * and a corresponding Method is set in `m_requestMethod`. If the method is not recognized,
 * an error code is set to 501 and a `std::runtime_error` is thrown.
 *
 * @param requestLine The request line string from which the method is to be parsed.
 * @return A substring of `requestLine` starting from the character after the parsed method.
 * @throws std::runtime_error If the method is not implemented, an error is thrown and the error code is set to 501.
 */
std::string RequestParser::parseMethod(const std::string& requestLine)
{
	int i = -1;
	while (isalpha(requestLine[++i]))
		m_request.method.push_back(requestLine[i]);

	if (m_request.method == "GET")
		m_requestMethod = MethodGet;
	else if (m_request.method == "POST")
		m_requestMethod = MethodPost;
	else if (m_request.method == "DELETE")
		m_requestMethod = MethodDelete;
	else {
		m_errorCode = 501;
		throw std::runtime_error(ERR_METHOD_NOT_IMPLEMENTED);
	}
	return (requestLine.substr(i));
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
 * @return A substring of `requestLine` starting from the character after the parsed URI.
 * @throws std::runtime_error If the URI is missing the initial slash,
 *         contains invalid characters, or any other URI-related errors occur.
 */
std::string RequestParser::parseUri(const std::string& requestLine)
{
	int i = 0;
	if (requestLine[i] != '/') {
		m_errorCode = 400;
		throw std::runtime_error(ERR_URI_MISS_SLASH);
	}
	m_request.uri.path.push_back(requestLine[i]);
	while (requestLine[++i]) {
		if (requestLine[i] == ' ')
			break;
		else if (!isValidURIChar(requestLine[i])) {
			m_errorCode = 400;
			throw std::runtime_error(ERR_URI_INVALID_CHAR);
		} else if (requestLine[i] == '?')
			parseUriQuery(requestLine, i);
		else if (requestLine[i] == '#')
			parseUriFragment(requestLine, i);
		else
			m_request.uri.path.push_back(requestLine[i]);
		// FIXME: setup max. URI length?
	}
	return (requestLine.substr(i));
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
 * @throws std::runtime_error If an invalid character is encountered in the query,
 *         or if another '?' is found, an error is thrown and the error code is set to 400.
 */
void RequestParser::parseUriQuery(const std::string& requestLine, int& index)
{
	while (requestLine[++index]) {
		if (requestLine[index] == ' ' || requestLine[index] == '#') {
			index--;
			break;
		} else if (!isValidURIChar(requestLine[index]) || requestLine[index] == '?') {
			m_errorCode = 400;
			throw std::runtime_error(ERR_URI_INVALID_CHAR);
		} else
			m_request.uri.query.push_back(requestLine[index]);
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
 * @throws std::runtime_error If an invalid character is encountered in the fragment,
 *         or if another '#' is found, an error is thrown and the error code is set to 400.
 */
void RequestParser::parseUriFragment(const std::string& requestLine, int& index)
{
	while (requestLine[++index]) {
		if (requestLine[index] == ' ') {
			index--;
			break;
		} else if (!isValidURIChar(requestLine[index]) || requestLine[index] == '#') {
			m_errorCode = 400;
			throw std::runtime_error(ERR_URI_INVALID_CHAR);
		} else
			m_request.uri.fragment.push_back(requestLine[index]);
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
 * @return A substring of `requestLine` starting from the character after the parsed version.
 * @throws std::runtime_error If the HTTP version format is invalid,
 *         an error is thrown and the error code is set to 400.
 */
std::string RequestParser::parseVersion(const std::string& requestLine)
{
	if (requestLine.substr(0, 5) != "HTTP/") {
		m_errorCode = 400;
		throw std::runtime_error(ERR_INVALID_VERSION_FORMAT);
	}
	int i = 5;
	if (!isdigit(requestLine[i])) {
		m_errorCode = 400;
		throw std::runtime_error(ERR_INVALID_VERSION_MAJOR);
	} else if (requestLine[i] != '1') {
        m_errorCode = 505;
        throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
    }
	m_request.version.push_back(requestLine[i]);
	if (requestLine[++i] != '.') {
		m_errorCode = 400;
		throw std::runtime_error(ERR_INVALID_VERSION_DELIM);
	}
	m_request.version.push_back(requestLine[i]);
	if (!isdigit(requestLine[++i])) {
		m_errorCode = 400;
		throw std::runtime_error(ERR_INVALID_VERSION_MINOR);
	} else if (requestLine[i] != '1' && requestLine[i] != '0') {
        m_errorCode = 505;
        throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
    }
	m_request.version.push_back(requestLine[i]);
	return (requestLine.substr(++i));
}

/**
 * @brief Parses a chunked body from the provided input stream.
 *
 * This function reads and processes the chunked transfer encoding format from the input stream.
 * It reads chunks of data prefixed by their size in hexadecimal format, appends the data to the
 * request body, and handles any formatting errors.
 *
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
void RequestParser::parseChunkedBody()
{
	int length = 0;
	std::string strChunkSize;
	std::string chunkData;
	size_t numChunkSize = 0;

	do {
		std::getline(m_requestStream, strChunkSize);
		if (strChunkSize[strChunkSize.size() - 1] == '\r')
			strChunkSize.erase(strChunkSize.size() - 1);
		else {
			m_errorCode = 400;
			throw std::runtime_error(ERR_MISS_CRLF);
		}
		numChunkSize = convertHex(strChunkSize);
		std::getline(m_requestStream, chunkData);
		if (chunkData[chunkData.size() - 1] == '\r')
			chunkData.erase(chunkData.size() - 1);
		else {
			m_errorCode = 400;
			throw std::runtime_error(ERR_MISS_CRLF);
		}
		if (chunkData.size() != numChunkSize) {
			m_errorCode = 400;
			throw std::runtime_error(ERR_CHUNK_SIZE);
		}
		m_request.body += chunkData;
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
void RequestParser::parseNonChunkedBody()
{
	std::string body;
	size_t length = 0;

	while (std::getline(m_requestStream, body)) {
		if (body[body.size() - 1] == '\r') {
			body.erase(body.size() - 1);
			length += 1;
		}
		if (!m_requestStream.eof())
			body += '\n';
		length += body.size();
		m_request.body += body;
	}
	size_t contentLength = std::atol(m_request.headers.at("Content-Length").c_str());
	if (contentLength != length) {
		m_errorCode = 400;
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
 * @throws std::runtime_error If the header name contains invalid characters or
 *         ends with whitespace, an error is thrown and the error code is set to 400.
 */
void RequestParser::checkHeaderName(const std::string& headerName)
{
	if (isspace(headerName[headerName.size() - 1])) {
		m_errorCode = 400;
		throw std::runtime_error(ERR_HEADER_COLON_WHITESPACE);
	}
	for (size_t i = 0; i < headerName.size(); i++) {
		if (!isValidHeaderFieldNameChar(headerName[i])) {
			m_errorCode = 400;
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
 * @throws std::runtime_error If there are multiple conflicting Content-Length
 *         headers or if the Content-Length value is invalid, an error is thrown
 *         and the error code is set to 400.
 */
void RequestParser::checkContentLength(const std::string& headerName, std::string& headerValue)
{
	if (headerName == "Content-Length") {
		if (headerValue.empty()) {
			m_errorCode = 400;
			throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
		}
		if (m_request.headers.find("Content-Length") != m_request.headers.end()
			&& m_request.headers["Content-Length"] != headerValue) {
			m_errorCode = 400;
			throw std::runtime_error(ERR_MULTIPLE_CONTENT_LENGTH_VALUES);
		}

		std::vector<std::string> strValues = split(headerValue, ',');
		std::vector<double> numValues;
		for (size_t i = 0; i < strValues.size(); i++) {
			char* endptr;
			double contentLength = strtod(strValues[i].c_str(), &endptr);
			if (!contentLength || *endptr != '\0') {
				m_errorCode = 400;
				throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
			}
			numValues.push_back(contentLength);
			if (i != 0 && contentLength != numValues[i - 1]) {
				m_errorCode = 400;
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
 * @throws std::runtime_error If the Transfer-Encoding header is missing or empty,
 *         or if the last encoding is not "chunked", an error is thrown and the error code is set to 400.
 */
void RequestParser::checkTransferEncoding()
{
	if (m_request.headers.find("Transfer-Encoding") != m_request.headers.end()) {
		if (m_request.headers.at("Transfer-Encoding").empty()) {
			m_errorCode = 400;
			throw std::runtime_error(ERR_NON_EXISTENT_TRANSFER_ENCODING);
		}

		if (m_request.headers.at("Transfer-Encoding").find("chunked") != std::string::npos) {
			std::vector<std::string> encodings = split(m_request.headers.at("Transfer-Encoding"), ',');
			if (encodings[encodings.size() - 1] != "chunked") {
				m_errorCode = 400;
				throw std::runtime_error(ERR_NON_FINAL_CHUNKED_ENCODING);
			}
			m_chunked = true;
			m_hasBody = true;
		}
	}
}
