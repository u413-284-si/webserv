#include "RequestParser.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

/**
 * @brief Constructs a new RequestParser object.
 *
 */
RequestParser::RequestParser() { }

/* ====== GETTERS/SETTERS ====== */

/**
 * Stateless class, no getters or setters.
 */

/* ====== MEMBER FUNCTIONS ====== */

/**
 * @brief Parses the header of an HTTP request.
 *
 * This function takes a string representation of an HTTP request header.
 * It extracts the request line and headers from the request string and populates the
 * provided HTTPRequest object with the parsed data.
 *
 * @param headerString The string representation of the HTTP request header.
 * @param request The HTTPRequest object to populate with the parsed data.
 *
 * @throws std::runtime_error If the request line or the headers do not conform to the RFC standards.
 */
void RequestParser::parseHeader(const std::string& headerString, HTTPRequest& request)
{
	m_requestStream.str(headerString);
	parseRequestLine(request);
	parseHeaders(request);
	if (isMultipartFormdata(request))
		extractBoundary(request);
	resetRequestStream();
}

/**
 * @brief Extracts the boundary string from the Content-Type header of an HTTP request.
 *
 * This function extracts the boundary string used in multipart/form-data requests
 * from the Content-Type header of the provided HTTP request. If the boundary string
 * is not found, it sets the HTTP status to Bad Request and indicates that the connection
 * should be closed.
 *
 * @param request The HTTP request from which to extract the boundary string.
 * @throws std::runtime_error if the boundary string is not found in the Content-Type header.
 */
void RequestParser::extractBoundary(HTTPRequest& request)
{
	const std::string denominator = "boundary=";

	std::string temp = request.headers.at("content-type");
	const size_t posBoundary = temp.find(denominator);

	if (posBoundary == std::string::npos) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_BAD_MULTIPART_FORMDATA);
	}

	request.boundary = temp.substr(posBoundary + denominator.size());

	LOG_DEBUG << "Extracted boundary string: " << request.boundary;
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
 * @brief Clears the contents of the RequestParser object.
 *
 * This function resets the internal request stream of the RequestParser object
 * by clearing the stream and setting the stream's internal string to an empty string.
 */
void RequestParser::resetRequestStream()
{
	m_requestStream.clear();
	m_requestStream.str("");
}

/* ====== REQUEST LINE PARSING ====== */

/**
 * @brief Parses the request line of an HTTP request.
 *
 * This function processes the first line of the HTTP request, which typically includes the method,
 * URI, and HTTP version. The request line is validated and broken down into its components.
 *
 * The function performs the following steps:
 * - Reads the request line from the request stream.
 * - Parses the HTTP method (e.g., GET, POST).
 * - Checks for and skips over required spaces between the components of the request line.
 * - Parses the request URI, which may include the path, query, and fragment.
 * - Parses the HTTP version.
 * - Validates that the line ends with a CRLF sequence.
 *
 * If any part of the request line is malformed or missing, the function sets the HTTP status
 * to `400 Bad Request` and throws an exception.
 *
 * @param request The HTTP request object where the parsed components of the request line will be stored.
 *
 * @throws std::runtime_error If the request line is missing, malformed, or contains invalid components.
 */
void RequestParser::parseRequestLine(HTTPRequest& request)
{
	std::string requestLine;
	if (!std::getline(m_requestStream, requestLine) || requestLine.empty()) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
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
 * After parsing the URI removeDotSegments() on parsed path is called.
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
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_URI_MISS_SLASH);
	}

	// Check URI string for invalid chars
	const std::string::const_iterator delimiterPos = find(requestLine.begin(), requestLine.end(), ' ');
	if (std::find_if(requestLine.begin(), delimiterPos, isNotValidURIChar) != delimiterPos) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
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
	}
	request.uri.path = decodePercentEncoding(request.uri.path, request);
	request.uri.fragment = decodePercentEncoding(request.uri.fragment, request);
	request.uri.query = decodePercentEncoding(request.uri.query, request);

	request.uri.path = removeDotSegments(request.uri.path, request);
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
			request.shallCloseConnection = true;
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
			request.shallCloseConnection = true;
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
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_INVALID_VERSION_FORMAT);
	}
	int index = versionPrefixLength;
	if (isdigit(requestLine[index]) == 0) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_INVALID_VERSION_MAJOR);
	}
	if (requestLine[index] != '1') {
		request.httpStatus = StatusNonSupportedVersion;
		throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
	}
	request.version.push_back(requestLine[index]);
	if (requestLine[++index] != '.') {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_INVALID_VERSION_DELIM);
	}
	request.version.push_back(requestLine[index]);
	if (isdigit(requestLine[++index]) == 0) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_INVALID_VERSION_MINOR);
	}
	if (requestLine[index] != '1' && requestLine[index] != '0') {
		request.httpStatus = StatusNonSupportedVersion;
		throw std::runtime_error(ERR_NONSUPPORTED_VERSION);
	}
	request.version.push_back(requestLine[index]);
	return (requestLine.substr(++index));
}

/* ====== HEADER PARSING ====== */

/**
 * @brief Parses the HTTP headers from the request stream.
 *
 * This function reads and parses the HTTP headers from the request stream until the end of the header section,
 * which is indicated by an empty line (`\r\n\r\n`). Each header is extracted, validated, and stored in the
 * `request` object.
 *
 * - If a header line begins with a space or tab, it is considered obsolete line folding, and the function
 *   sets the HTTP status to `400 Bad Request` and throws an exception.
 * - The function splits each header into a name and value pair using the `:` delimiter, trims any leading
 *   or trailing whitespace, and stores the pair in the `request.headers` map.
 * - The `Content-Length` and `Transfer-Encoding` headers are checked specifically for validity.
 *
 * @param request The HTTP request object where the parsed headers will be stored.
 *
 * @throws std::runtime_error If the header line contains obsolete line folding or if there's an issue
 * with the `Content-Length` header.
 */
void RequestParser::parseHeaders(HTTPRequest& request)
{
	std::string headerLine;

	// The end of the headers section is marked by an empty line (\r\n\r\n).
	while (!std::getline(m_requestStream, headerLine).fail() && headerLine != "\r" && !headerLine.empty()) {
		if (headerLine[0] == ' ' || headerLine[0] == '\t') {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_OBSOLETE_LINE_FOLDING);
		}
		std::string headerName;
		std::string headerValue;
		const std::size_t delimiterPos = headerLine.find_first_of(':');
		if (delimiterPos != std::string::npos) {
			headerName = headerLine.substr(0, delimiterPos);
			validateHeaderName(headerName, request);
			webutils::lowercase(headerName);
			headerValue = headerLine.substr(delimiterPos + 1);
			if (headerValue[headerValue.size() - 1] == '\r')
				headerValue.erase(headerValue.size() - 1);
			headerValue = webutils::trimLeadingWhitespaces(headerValue);
			webutils::trimTrailingWhiteSpaces(headerValue);
			validateContentLength(headerName, headerValue, request);
			validateNoMultipleHostHeaders(headerName, request);
			request.headers[headerName] = headerValue;
			LOG_DEBUG << "Parsed header: " << headerName << " -> " << headerValue;
		}
	}
	validateHostHeader(request);
	validateTransferEncoding(request);
	validateMethodWithBody(request);
}

/* ====== BODY PARSING ====== */

/**
 * @brief Parses the chunked body of an HTTP request.
 *
 * This function processes the chunked transfer encoding body of the provided HTTP request.
 * It handles partial chunk sizes and chunk data, retaining the last parsed position between calls.
 * The function updates the request body as chunks are parsed and sets the appropriate HTTP status
 * if any errors are encountered.
 *
 * @param bodyBuffer The buffer containing the chunked body data.
 * @param request The HTTP request object to be filled with the parsed body data.
 * @throws std::runtime_error if the chunk size is too large or if the chunk data is malformed.
 */
void RequestParser::parseChunkedBody(const std::string& bodyBuffer, HTTPRequest& request)
{
	LOG_DEBUG << "Parsing chunked body...";
	size_t length = 0;

	while (request.currParsingPos < bodyBuffer.size()) {

		// Step 1: Parse chunk size
		if (request.chunkSize == -1) { // Not currently parsing a chunk
			const size_t newlinePos = bodyBuffer.find("\r\n", request.currParsingPos);
			if (newlinePos == std::string::npos) // Incomplete chunk size indication, wait for more data
				return;

			std::string strChunkSize = bodyBuffer.substr(request.currParsingPos, newlinePos - request.currParsingPos);
			request.currParsingPos = newlinePos + 2; // Move past \r\n

			request.chunkSize = convertHex(strChunkSize);
			if (request.chunkSize > s_maxChunkSize) {
				request.httpStatus = StatusRequestEntityTooLarge;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_TOO_LARGE_CHUNKSIZE);
			}

			if (request.chunkSize == 0) { // Zero chunk size indicates the end of the body
				if (bodyBuffer.size() - request.currParsingPos != 2 || bodyBuffer.at(request.currParsingPos) != '\r'
					|| bodyBuffer.at(request.currParsingPos + 1) != '\n') { // Check for final CRLF
					request.httpStatus = StatusBadRequest;
					request.shallCloseConnection = true;
					throw std::runtime_error(ERR_MISS_CRLF);
				}
				request.isCompleteBody = true;
				request.headers["content-length"] = webutils::toString(length);
				LOG_DEBUG << "Successfully parsed chunked body";
				return;
			}
		}

		// Step 2: Parse chunk data
		const size_t remainingData = bodyBuffer.size() - request.currParsingPos;
		const size_t requiredData = request.chunkSize + 2; // Chunk data + \r\n

		if (remainingData < requiredData) // Incomplete chunk data, wait for more data
			return;

		if (bodyBuffer.at(request.currParsingPos + request.chunkSize) != '\r'
			|| bodyBuffer.at(request.currParsingPos + request.chunkSize + 1) != '\n') {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_MISS_CRLF);
		}

		request.body.append(bodyBuffer, request.currParsingPos, request.chunkSize);
		request.currParsingPos += requiredData; // Move past the current chunk
		length += request.chunkSize;

		// Reset for next chunk
		request.chunkSize = -1;
	}
}

/**
 * @brief Decodes the multipart/form-data content of an HTTP request.
 *
 * This function processes the multipart/form-data content of the provided HTTP request.
 * It extracts the filename and appends it to the target resource.
 * The extracted content from the request body is then stored back into the request body.
 *
 * @param request The HTTP request containing the multipart/form-data content to decode.
 * @throws std::runtime_error if the format of the request is invalid.
 */
void RequestParser::decodeMultipartFormdata(HTTPRequest& request)
{
	const std::string filename = "filename=\"";
	size_t filenameStartPos = checkForString(filename, 0, request);
	filenameStartPos += filename.size();
	size_t filenameEndPos = checkForString("\"", filenameStartPos, request);
	request.targetResource += request.body.substr(filenameStartPos, filenameEndPos - filenameStartPos);
	LOG_DEBUG << "New target resource: " << request.targetResource;

	size_t contentTypePos = checkForString("Content-Type:", filenameEndPos, request);

	size_t contentStartPos = checkForString("\r\n\r\n", contentTypePos, request);
	contentStartPos += 4;

	const std::string endBoundary = "--" + request.boundary + "--";
	size_t contentEndPos = checkForString(endBoundary, contentStartPos, request);
	contentEndPos -= 2; // Remove the CRLF at the end

	request.body = request.body.substr(contentStartPos, contentEndPos - contentStartPos);
}

/* ====== CHECKS ====== */

/**
 * @brief Checks for the presence of a specific string in the HTTP request body starting from a given position.
 *
 * This function searches for the specified string in the body of the provided HTTP request,
 * starting from the given position. If the string is not found, it sets the HTTP status to Bad Request,
 * indicates that the connection should be closed, and throws a runtime error.
 *
 * @param string The string to search for in the request body.
 * @param startPos The position in the request body to start the search from.
 * @param request The HTTP request containing the body to search.
 * @return The position of the found string in the request body.
 * @throws std::runtime_error if the string is not found in the request body.
 */
size_t RequestParser::checkForString(const std::string& string, size_t startPos, HTTPRequest& request)
{
	size_t pos = request.body.find(string, startPos);
	if (pos == std::string::npos) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_BAD_MULTIPART_FORMDATA);
	}
	return pos;
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
void RequestParser::validateHeaderName(const std::string& headerName, HTTPRequest& request)
{
	if (isspace(headerName[headerName.size() - 1]) != 0) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_HEADER_COLON_WHITESPACE);
	}
	for (size_t i = 0; i < headerName.size(); i++) {
		if (!isValidHeaderFieldNameChar(headerName[i])) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
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
void RequestParser::validateContentLength(const std::string& headerName, std::string& headerValue, HTTPRequest& request)
{
	if (headerName == "content-length") {
		if (headerValue.empty()) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
		}
		if (request.headers.find("content-length") != request.headers.end()
			&& request.headers["content-length"] != headerValue) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_MULTIPLE_CONTENT_LENGTH_VALUES);
		}

		std::vector<std::string> strValues = webutils::split(headerValue, ", ");
		std::vector<unsigned long> numValues;
		for (size_t i = 0; i < strValues.size(); i++) {
			char* endptr = NULL;
			request.contentLength = std::strtoul(strValues[i].c_str(), &endptr, constants::g_decimalBase);
			if ((request.contentLength == 0) || *endptr != '\0') {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_INVALID_CONTENT_LENGTH);
			}
			numValues.push_back(request.contentLength);
			if (i != 0 && request.contentLength != numValues[i - 1]) {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_MULTIPLE_CONTENT_LENGTH_VALUES);
			}
		}
		request.hasBody = true;
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
 *         or if the last encoding is not "chunked", an error is thrown and the error code is set to
 * StatusBadRequest.
 */
void RequestParser::validateTransferEncoding(HTTPRequest& request)
{
	LOG_DEBUG << "Validating Transfer-Encoding header...";

	if (request.headers.find("transfer-encoding") != request.headers.end()) {
		if (request.headers.at("transfer-encoding").empty()) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_NON_EXISTENT_TRANSFER_ENCODING);
		}
		if (request.headers.find("content-length") != request.headers.end())
			request.shallCloseConnection = true;

		if (request.headers.at("transfer-encoding").find("chunked") != std::string::npos) {
			std::vector<std::string> encodings = webutils::split(request.headers.at("transfer-encoding"), ", ");
			if (encodings[encodings.size() - 1] != "chunked") {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_NON_FINAL_CHUNKED_ENCODING);
			}
			request.isChunked = true;
			request.hasBody = true;
		}
	}

	LOG_DEBUG << "Transfer-Encoding header is valid.";
}

/**
 * @brief Validates if the HTTP request method is allowed to have a body.
 *
 * This function checks if the HTTP request contains a body and whether the
 * method used in the request is allowed to have a body. If the request has a
 * body but the method is not allowed to have one, it sets the HTTP status to
 * StatusMethodNotAllowed and throws a runtime error.
 *
 * @param request The HTTPRequest object to be validated.
 *
 * @throws std::runtime_error If the request has a body but the method is not
 *         allowed to have a body.
 */
void RequestParser::validateMethodWithBody(HTTPRequest& request)
{
	LOG_DEBUG << "Validating method with body...";

	if (request.hasBody && !isMethodAllowedToHaveBody(request)) {
		request.httpStatus = StatusMethodNotAllowed;
		throw std::runtime_error(ERR_UNEXPECTED_BODY);
	}

	LOG_DEBUG << "Method with body is valid.";
}

/**
 * @brief Validates the "Host" header in the given HTTP request.
 *
 * This function checks if the "Host" header is present and non-empty in the HTTP request.
 * It also validates the format of the host value, ensuring it is either a valid IP address
 * (with or without a port) or a valid hostname.
 *
 * @param request The HTTP request object containing headers to be validated.
 *
 * @throws std::runtime_error If the "Host" header is missing, empty, or contains an invalid value.
 * The specific error message thrown depends on the type of validation failure:
 * - ERR_MISSING_HOST_HEADER: The "Host" header is missing.
 * - ERR_EMPTY_HOST_VALUE: The "Host" header value is empty.
 * - ERR_INVALID_HOST_IP_WITH_PORT: The "Host" header contains an invalid IP address with a port.
 * - ERR_INVALID_HOST_IP: The "Host" header contains an invalid IP address.
 * - ERR_INVALID_HOSTNAME: The "Host" header contains an invalid hostname.
 */
void RequestParser::validateHostHeader(HTTPRequest& request)
{
	LOG_DEBUG << "Validating Host header...";

	std::map<std::string, std::string>::const_iterator iter = request.headers.find("host");
	if (iter == request.headers.end()) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_MISSING_HOST_HEADER);
	}

	if (iter->second.empty()) {
		request.httpStatus = StatusBadRequest;
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_EMPTY_HOST_VALUE);
	}

	if (iter->second.find(':') != std::string::npos) {
		if (!webutils::isIpAddressValid(iter->second.substr(0, iter->second.find(':')))
			|| !webutils::isPortValid(iter->second.substr(iter->second.find(':') + 1))) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_INVALID_HOST_IP_WITH_PORT);
		}
	} else if (iter->second.find_first_not_of("0123456789.") == std::string::npos) {
		if (!webutils::isIpAddressValid(iter->second.substr(0, iter->second.find(':')))) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_INVALID_HOST_IP);
		}
	} else {
		if (!isValidHostname(iter->second)) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_INVALID_HOSTNAME);
		}
	}

	LOG_DEBUG << "Valid host header: " << iter->second;
}

/**
 * @brief Validates that there are no multiple "Host" headers in the HTTP request.
 *
 * This function checks if the provided header name is "Host". If it is, it then checks
 * if the "Host" header already exists in the request headers. If a "Host" header is
 * found, it sets the HTTP status of the request to BadRequest and throws a runtime error.
 *
 * @param headerName The name of the header to validate.
 * @param request The HTTP request object containing the headers.
 *
 * @throws std::runtime_error if multiple "Host" headers are found.
 */
void RequestParser::validateNoMultipleHostHeaders(const std::string& headerName, HTTPRequest& request)
{
	if (headerName == "host") {
		if (request.headers.find("host") != request.headers.end()) {
			request.httpStatus = StatusBadRequest;
			request.shallCloseConnection = true;
			throw std::runtime_error(ERR_MULTIPLE_HOST_HEADERS);
		}
	}
}

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
	request.shallCloseConnection = true;
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
		request.shallCloseConnection = true;
		throw std::runtime_error(ERR_MISS_CRLF);
	}
}

/**
 * @brief Checks if a given character is a valid URI character.
 *
 * This function determines if the provided character `character` is a valid character in a URI.
 * It checks against both unreserved and reserved characters as per the URI specification.
 * It also accepts `%` as encoding character.
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

	// Check for reserved characters and encoding char `%`
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
	case '%':
		return false;
	default:
		return true;
	}
}

/**
 * @brief Checks if a given character is a valid HTTP header field name character.
 *
 * This function determines if the provided character `character` is valid for use in an HTTP header field name.
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
 * @return The converted long value.
 *
 * @throws std::invalid_argument if the chunkSize string is empty or contains invalid hexadecimal characters.
 * @throws std::runtime_error if the conversion from string to size_t fails.
 */
long RequestParser::convertHex(const std::string& chunkSize)
{
	if (chunkSize.empty())
		throw std::invalid_argument(ERR_NON_EXISTENT_CHUNKSIZE);

	for (std::string::const_iterator it = chunkSize.begin(); it != chunkSize.end(); ++it) {
		if (std::isxdigit(*it) == 0)
			throw std::invalid_argument(ERR_INVALID_HEX_CHAR);
	}

	std::istringstream iss(chunkSize);
	long value = 0;

	iss >> std::hex >> value;
	if (iss.fail())
		throw std::runtime_error(ERR_CONVERSION_STRING_TO_HEX);

	LOG_DEBUG << "Converted hex value: " << value;
	return value;
}

/**
 * @brief Checks if the HTTP request method allows a body.
 *
 * This function determines whether the specified HTTP request method can have
 * a body. The methods POST and DELETE allow bodies, while method GET does not.
 *
 * @param request The HTTP request object containing the method to check.
 *
 * @return true If the method allows a body (e.g., POST, DELETE).
 * @return false If the method does not allow a body (e.g., GET).
 *
 * @note The function asserts that the method in the request is within the
 *       expected range (from MethodGet to MethodCount).
 */
bool RequestParser::isMethodAllowedToHaveBody(HTTPRequest& request)
{
	assert(request.method >= MethodGet && request.method <= MethodCount);

	switch (request.method) {
	case MethodGet:
	case MethodCount:
		return false;
	case MethodPost:
	case MethodDelete:
		return true;
	}
	// this is never reached
	return false;
}

/**
 * @brief Checks if a character is valid in a hostname and updates the alpha flag.
 *
 * This function determines if the given character is a valid part of a hostname.
 * A valid hostname character is either an alphanumeric character or a hyphen ('-').
 * Additionally, if the character is an alphabetic character, the function sets the
 * hasAlpha flag to true.
 *
 * @param character The character to be checked.
 * @param hasAlpha A reference to a boolean flag that will be set to true if the character is alphabetic.
 * @return true if the character is a valid hostname character, false otherwise.
 */
bool RequestParser::isValidHostnameChar(char character, bool& hasAlpha)
{
	if (std::isalpha(character) != 0)
		hasAlpha = true;
	return ((std::isalnum(character) != 0) || character == '-');
}

/**
 * @brief Validates a label within a hostname.
 *
 * This function checks if a given label within a hostname is valid according to the following rules:
 * - The label must not be empty and must not exceed the maximum label length (RFC 1035).
 * - The label must not start or end with a hyphen ('-').
 * - All characters in the label must be valid hostname characters (alphanumeric or hyphen).
 *
 * @param label The label string to be validated.
 * @return true if the label is valid, false otherwise.
 */
bool RequestParser::isValidLabel(const std::string& label, bool& hasAlpha)
{
	if (label.empty() || label.length() > s_maxLabelLength)
		return false;

	// Label must not start or end with a hyphen
	if (label.at(0) == '-' || label.at(label.length() - 1) == '-')
		return false;

	for (size_t i = 0; i < label.length(); ++i)
		if (!isValidHostnameChar(label.at(i), hasAlpha))
			return false;
	return true;
}

/**
 * @brief Validates if the given hostname is valid according to specific rules.
 *
 * This function checks if the hostname length does not exceed the maximum allowed length
 * and if each label (substring between periods) within the hostname is valid.
 * Also verifies that the hostname has at least one alphabetic character.
 *
 * @param hostname The hostname to be validated.
 * @return true if the hostname is valid, false otherwise.
 */
bool RequestParser::isValidHostname(const std::string& hostname)
{
	if (hostname.length() > s_maxHostNameLength)
		return false;

	size_t labelStart = 0;
	size_t labelEnd = 0;
	bool hasAlpha = false;

	// Split hostname by periods and validate each label
	while (labelEnd != std::string::npos) {
		labelEnd = hostname.find('.', labelStart);

		std::string label = (labelEnd == std::string::npos) ? hostname.substr(labelStart)
															: hostname.substr(labelStart, labelEnd - labelStart);
		if (!isValidLabel(label, hasAlpha))
			return false;
		labelStart = labelEnd + 1;
	}
	return hasAlpha;
}

/**
 * @brief Decodes a percent encoded string.
 *
 * Iterates through the string. If it finds a '%' it converts the next two chars to hex and appends the char to the
 * buffer. Then jumps over the triplet. Else it just adds the char to the buffer.
 * Sets HTTP status to StatusBadRequest, shallCloseConnection to true, and throws if
 * - '%' is not followed by two chars.
 * - "%00" is encountered ('\0' or NUL terminator). This char is not supported.
 * - a percent encoded triplet consists of non-hex chars.
 *
 * According to RFV 3986 Sect 2.1. percent encoding is used to represent a char which is outside of the allowed set.
 * This could either be a unicode character like 'ö' or a reserved char with special meaning. Char is encoded with a
 * triplet consisting of percent char '%' followed by the two hexadecimal digits representing the chars numeric value.
 * @param encoded The percent encoded string.
 * @param request The HTTP request object to be filled.
 * @return std::string The decoded string.
 * @throws std::runtime_error if invalid "%00" is encountered or '%' is not followed by two chars.
 * @sa https://datatracker.ietf.org/doc/html/rfc3986#section-2.1
 */
std::string RequestParser::decodePercentEncoding(const std::string& encoded, HTTPRequest& request)
{
	std::string decoded;
	std::string::const_iterator iter = encoded.begin();

	while (iter != encoded.end()) {
		if (*iter == '%') {
			if (std::distance(iter, encoded.end()) < 3) {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_PERCENT_INCOMPLETE);
			}
			if ((std::isxdigit(*(iter + 1)) == 0) || (std::isxdigit(*(iter + 2)) == 0)) {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_PERCENT_INVALID_HEX);
			}
			std::string hex = std::string(iter + 1, iter + 3);
			unsigned int value = 0;
			std::istringstream(hex) >> std::hex >> value;
			if (value == 0) {
				request.httpStatus = StatusBadRequest;
				request.shallCloseConnection = true;
				throw std::runtime_error(ERR_PERCENT_NONSUPPORTED_NUL);
			}
			decoded += static_cast<char>(value);
			iter += 3;
		} else {
			decoded += *iter;
			++iter;
		}
	}

	return decoded;
}

/**
 * @brief Checks for a single dot segment "/."
 *
 * Needs to check if iterator points to end, as derefencing end iterator is undefined behavior.
 * @param iter Iterator to the current position.
 * @param end End iterator of string.
 * @return true It is a single dot segment.
 * @return false It is not a single dot segment.
 */
static bool isSingleDot(const std::string::const_iterator& iter, const std::string::const_iterator& end)
{
	if (iter == end)
		return false;
	return *iter == '.' && (iter + 1 == end || *(iter + 1) == '/');
}

/**
 * @brief Checks for a double dot segment "/.."
 *
 * Needs to check if iterator or iterator + 1 point to end, as derefencing end iterator is undefined behavior.
 *
 * @param iter Iterator to the current position.
 * @param end End iterator of string.
 * @return true It is a double dot segment.
 * @return false It is not a double dot segment.
 */
static bool isDoubleDot(const std::string::const_iterator& iter, const std::string::const_iterator& end)
{
	if (iter == end || iter + 1 == end)
		return false;
	return *iter == '.' && *(iter + 1) == '.' && (iter + 2 == end || *(iter + 2) == '/');
}

/**
 * @brief Removes last segment of a path denoted by '/'.
 *
 * Output buffer always has at least one '/': a request needs to start with '/' per RFC 9110. Function is not entered if
 * output is empty.
 * @param output Reference to buffer containing path.
 */
static void removeLastSegment(std::string& output)
{
	size_t lastSlash = output.find_last_of('/');
	output.erase(lastSlash);
}

/**
 * @brief Removes dot segments "/." and "/.." from a path.
 *
 * Iterates through the string. If it finds a '/', increments and checks for
 * - isSingleDot() = "/." - increment to ignore dot segment.
 * - isDoubleDot() = "/.." - if output buffer is empty throw and set status to BadRequest since it travels outside of
 * root. Else increment to ignore double dot segment and removeLastSegment() of output.
 * - else - append a '/' to output buffer.
 * If not a '/' append to buffer.
 *
 * This implements the algorithm as described in RFC 3986 Sect. 5.2.4.
 * However it does not handle all described algorithm conditions
 * - 2.A. buffer begins with "./" or "../" (note: '.' comes before '/')
 * - 2.D. buffer consists only of '.' or '..'
 * As requests need an absolute path (starting with '/') per RFC 9110 we can ignore these conditions.
 *
 * @sa https://datatracker.ietf.org/doc/html/rfc3986#section-5.2.4
 * @param path Where to remove possible dot segments from.
 * @return std::string String with dot segments removed.
 * @throws std::runtime_error If double dot segment is encountered while output is empty.
 */
std::string RequestParser::removeDotSegments(const std::string& path, HTTPRequest& request)
{
	std::string output;
	std::string::const_iterator iter = path.begin();

	while (iter != path.end()) {
		if (*iter == '/') {
			iter++;

			if (isSingleDot(iter, path.end())) {
				++iter;
				continue;
			}

			if (isDoubleDot(iter, path.end())) {
				if (output.empty()) {
					request.httpStatus = StatusBadRequest;
					request.shallCloseConnection = true;
					throw std::runtime_error(ERR_DIRECTORY_TRAVERSAL);
				}
				iter += 2;
				removeLastSegment(output);
				continue;
			}

			output += '/';
		} else {
			output += *iter;
			iter++;
		}
	}
	return output.empty() ? "/" : output;
}

/**
 * @brief Checks if the HTTP request contains multipart/form-data content.
 *
 * This function checks the Content-Type header of the provided HTTP request
 * to determine if it contains multipart/form-data content. If the content type
 * is multipart/form-data, it sets the hasMultipartFormdata flag in the request
 * and logs the detection.
 *
 * @param request The HTTP request to check.
 * @return true if the request contains multipart/form-data content, false otherwise.
 */
bool RequestParser::isMultipartFormdata(HTTPRequest& request)
{
	if (request.headers.find("content-type") != request.headers.end()
		&& request.headers["content-type"].find("multipart/form-data") != std::string::npos) {
		request.hasMultipartFormdata = true;
		LOG_DEBUG << "Multipart/form-data detected";
		return true;
	}
	return false;
}
