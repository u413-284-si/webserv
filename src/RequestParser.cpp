#include "RequestParser.hpp"

/* ====== CONSTRUCTOR/DESTRUCTOR ====== */

RequestParser::RequestParser() {}

RequestParser::~RequestParser() {}

/* ====== MEMBER FUNCTIONS ====== */

HTTPRequest	RequestParser::parseHttpRequest(const std::string& request)
{
	std::istringstream	requestStream(request);
	
	// Step 1: Parse the request-line
	std::string			requestLine;
	if (!std::getline(requestStream, requestLine) || requestLine.empty())
		throw std::runtime_error("Invalid HTTP request: missing request line");
	
	std::istringstream	requestLineStream(requestLine);
    if (!(requestLineStream >> m_request.method >> m_request.uri >> m_request.version))
        throw std::runtime_error("Invalid HTTP request: malformed request line");
	parseMethod();
	parseUri();
	parseVersion();

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

void	RequestParser::parseMethod() const {
	if (m_request.method == "GET")
}
