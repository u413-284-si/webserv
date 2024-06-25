#pragma once

/* ====== LIBRARIES ====== */

#include "utilities.hpp"
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>
#include <stdint.h>
#include <vector>

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
	bool								hasBody;
	bool								chunked;
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
				void		checkHeaderName(const std::string& headerName);
				void		checkContentLength(const std::string& headerName, std::string& headerValue);
				void		checkTransferEncoding();

				// Helper functions
				std::string	checkForSpace(const std::string&);
				void		checkForCRLF(const std::string&);
				bool		isValidURIChar(uint8_t c) const;
				bool		isValidHeaderFieldNameChar(uint8_t c) const;

				// Getter functions
				int			getErrorCode() const;
				int			getRequestMethod() const;

	private:
				int			m_errorCode;
				int			m_requestMethod;
				HTTPRequest	m_request;
};
