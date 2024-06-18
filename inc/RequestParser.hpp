#pragma once

/* ====== LIBRARIES ====== */

#include "utilities.hpp"
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>
#include <stdint.h>

/* ====== DEFINITIONS ====== */

struct URI {
	std::string		path;
	std::string		query;
	std::string		fragment;
};

struct HTTPRequest {
	std::string							method;
	URI									uri;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
};

/* ====== CLASS DECLARATION ====== */

class RequestParser {
	public:
				RequestParser();
				~RequestParser();

				HTTPRequest	parseHttpRequest(const std::string& request);
				std::string	parseMethod(const std::string& requestLine);
				std::string	parseUri(const std::string& requestLine);
				void		parseUriQuery(const std::string& requestLine, int& index);
				void		parseUriFragment(const std::string& requestLine, int& index);
				std::string	parseVersion(const std::string& requestLine);
				void		parseHeaderValue(const std::string& headerValue);

				// Helper functions
				std::string	checkForSpace(const std::string&);
				void		checkForCRLF(const std::string&);
				bool		isValidURIChar(uint8_t c) const;
				bool		isValidHeaderFieldValueToken(uint8_t c) const; 

	private:
				int			m_errorCode;
				int			m_requestMethod;
				HTTPRequest	m_request;
};
