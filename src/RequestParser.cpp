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

    // Step 2: Parse headers
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r" && !headerLine.empty()) {
        std::istringstream headerStream(headerLine);
        std::string headerName;
        std::string headerValue;
        if (std::getline(headerStream, headerName, ':')) {
            std::getline(headerStream >> std::ws, headerValue);
            httpRequest.headers[headerName] = headerValue;
        }
    }

    // Parse body (if any)
    std::string body;
    while (std::getline(requestStream, body)) {
        httpRequest.body += body + "\n";
    }
    if (!httpRequest.body.empty() && httpRequest.body.back() == '\n') {
        httpRequest.body.pop_back(); // Remove the last newline character
    }

    return httpRequest;
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

std::string	RequestParser::parseUri(const std::string& requestLine) {

}