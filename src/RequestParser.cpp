#include "RequestParser.hpp"

/* ====== HELPER FUNCTIONS ====== */

std::string	RequestParser::checkForSpace(const std::string& str) {
	if (str.length() > 1 && str[0] == ' ' && str[1] != ' ')
		return (str.substr(1));
	else {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: missing single space");
	}
}

/**
 * @brief Check for CRLF at the end of the request line.
 * 
 * Only checks for the carriage return \r, as the newline \n is discarded
 * by std::getline
 * @param str	String containing remainder of request line after HTTP version
 */
void	RequestParser::checkForCRLF(const std::string& str) {
	if (str.length() != 1 || str[0] != '\r') {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: missing CRLF");
	}
}

bool	RequestParser::isValidURIChar(uint8_t c) const {
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

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

RequestParser::RequestParser() {}

RequestParser::~RequestParser() {}

/* ====== MEMBER FUNCTIONS ====== */

HTTPRequest	RequestParser::parseHttpRequest(const std::string& request)
{
	std::istringstream	requestStream(request);
	
	// Step 1: Parse the request-line
	std::string			requestLine;
	if (!std::getline(requestStream, requestLine) || requestLine.empty()) {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: missing request line");
	}
	requestLine = parseMethod(requestLine);
	requestLine = checkForSpace(requestLine);
	requestLine = parseUri(requestLine);
	requestLine = checkForSpace(requestLine);
	requestLine = parseVersion(requestLine);
	checkForCRLF(requestLine);

    // Step 2: Parse headers
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r" && !headerLine.empty()) {
        std::istringstream headerStream(headerLine);
        std::string headerName;
        std::string headerValue;
        if (std::getline(headerStream, headerName, ':')) {
            std::getline(headerStream >> std::ws, headerValue);
			headerValue = trimTrailingWhiteSpaces(headerValue);
            m_request.headers[headerName] = headerValue;
        }
    }

    // Step 3: Parse body (if any)
    std::string body;
    while (std::getline(requestStream, body)) {
        if (!m_request.body.empty())
			body += "\n";
		m_request.body += body;
    }
    return m_request;
}

std::string	RequestParser::parseMethod(const std::string& requestLine) {
	int	i = -1;
	while (isalpha(requestLine[++i]))
		m_request.method.push_back(requestLine[i]);

	if (m_request.method == "GET")
		m_requestMethod = 1 << 0;
	else if (m_request.method == "POST")
		m_requestMethod = 1 << 1;
	else if (m_request.method == "DELETE")
		m_requestMethod = 1 << 2;
	else {
		m_errorCode = 501;
		throw std::runtime_error("Invalid HTTP request: method not implemented");
	}
	return (requestLine.substr(i));
}

/**
 * @brief Parse URI for origin server
 * 
 * used to identify a resource on an origin server or gateway.
 * 
 * @param requestLine 
 * @return std::string 
 */
std::string	RequestParser::parseUri(const std::string& requestLine) {
	int	i = 0;
	if (requestLine[i] != '/') {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: missing slash in URI");
	}
	m_request.uri.path.push_back(requestLine[i]);
	while (requestLine[++i]) {
		if (requestLine[i] == ' ')
			break;
		else if (!isValidURIChar(requestLine[i])) {
			m_errorCode = 400;
			throw std::runtime_error("Invalid HTTP request: invalid char in URI");
		}
		else if (requestLine[i] == '?')
			parseUriQuery(requestLine, i);
		else if (requestLine[i] == '#')
			parseUriFragment(requestLine, i);
		else
			m_request.uri.path.push_back(requestLine[i]);
		// FIXME: setup max. URI length?
	}
	return (requestLine.substr(i));
}

void	RequestParser::parseUriQuery(const std::string& requestLine, int& index) {
	while (requestLine[++index]) {
		if (requestLine[index] == ' ' || requestLine[index] == '#') {
			index--;
			break;
		}
		else if (!isValidURIChar(requestLine[index]) || requestLine[index] == '?') {
			m_errorCode = 400;
			throw std::runtime_error("Invalid HTTP request: invalid char in URI");
		}
		else
			m_request.uri.query.push_back(requestLine[index]);
	}
}

void	RequestParser::parseUriFragment(const std::string& requestLine, int& index) {
	while (requestLine[++index]) {
		if (requestLine[index] == ' ') {
			index--;
			break;
		}
		else if (!isValidURIChar(requestLine[index]) || requestLine[index] == '#') {
			m_errorCode = 400;
			throw std::runtime_error("Invalid HTTP request: invalid char in URI");
		}
		else
			m_request.uri.fragment.push_back(requestLine[index]);
	}
}

std::string	RequestParser::parseVersion(const std::string& requestLine) {
	if (requestLine.substr(0, 5) != "HTTP/") {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: invalid format of version");
	}
	int	i = 5;
	if (!isdigit(requestLine[i])) {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: invalid version major");
	}
	m_request.version.push_back(requestLine[i]);
	if (requestLine[++i] != '.') {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: invalid version delimiter");
	}
	m_request.version.push_back(requestLine[i]);
	if (!isdigit(requestLine[++i])) {
		m_errorCode = 400;
		throw std::runtime_error("Invalid HTTP request: invalid version minor");
	}
	m_request.version.push_back(requestLine[i]);
	return (requestLine.substr(++i));
}
