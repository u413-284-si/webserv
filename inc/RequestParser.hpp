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
				HTTPRequest	m_request;
				int			m_requestMethod = 0;

				RequestParser();
				~RequestParser();

				HTTPRequest	parseHttpRequest(const std::string& request);
				void		parseMethod();
				void		parseUri() const;
				void		parseVersion() const;

	private:
				
};
