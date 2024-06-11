#pragma once

/* ====== LIBRARIES ====== */

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <sstream>

/* ====== DEFINITIONS ====== */

struct HTTPRequest {
	std::string										method;
	URI												uri;
	std::string										version;
	std::unordered_map<std::string, std::string>	headers;
	std::string										body;
};

struct URI {
	std::string		path;
	std::string		query;
	std::string		fragment;
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

				// Helper functions
				std::string	checkForSpace(const std::string&);
				void		checkForCRLF(const std::string&);
				bool		isValidURIChar(uint8_t c) const;

	private:
				int			m_errorCode;
				int			m_requestMethod;
				HTTPRequest	m_request;
};
