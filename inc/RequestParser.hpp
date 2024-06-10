#pragma once

/* ====== LIBRARIES ====== */

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sstream>

/* ====== DEFINITIONS ====== */

struct HTTPRequest {
	std::string										method;
	std::string										uri;
	std::string										version;
	std::unordered_map<std::string, std::string>	headers;
	std::string										body;
};

/* ====== CLASS DECLARATION ====== */

class RequestParser {
	public:
				RequestParser();
				~RequestParser();

				HTTPRequest	parseHttpRequest(const std::string& request);
				std::string	parseMethod(const std::string& requestLine);
				std::string	parseUri(const std::string& requestLine);
				std::string	parseVersion(const std::string& requestLine);

				// Helper functions
				std::string	checkForSpace(const std::string&);
				bool		isValidURIChar(uint8_t c) const;

	private:
				int			m_errorCode;
				int			m_requestMethod = 0;
				HTTPRequest	m_request;
};
