#pragma once

/* ====== LIBRARIES ====== */

#include "utilities.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>
#include <stdint.h>
#include <vector>

/* ====== DEFINITIONS ====== */

/* ====== CLASS DECLARATION ====== */

class RequestParser {
	public:
				RequestParser();
				~RequestParser();

				HTTPRequest	parseHttpRequest(const std::string& request);
			
				// Getter functions
				int			getErrorCode() const;
				int			getRequestMethod() const;

	private:
				int			m_errorCode;
				int			m_requestMethod;
				HTTPRequest	m_request;

				std::string	parseMethod(const std::string& requestLine);
				std::string	parseUri(const std::string& requestLine);
				void		parseUriQuery(const std::string& requestLine, int& index);
				void		parseUriFragment(const std::string& requestLine, int& index);
				std::string	parseVersion(const std::string& requestLine);
				void		checkHeaderName(const std::string& headerName);
				void		checkContentLength(const std::string& headerName, std::string& headerValue);
				void		checkTransferEncoding();
				void		parseChunkedBody(std::istringstream& requestStream);
				void		parseNonChunkedBody(std::istringstream& requestStream);

				// Helper functions
				std::string	checkForSpace(const std::string&);
				void		checkForCRLF(const std::string&);
				bool		isValidURIChar(uint8_t c) const;
				bool		isValidHeaderFieldNameChar(uint8_t c) const;
				size_t		convertHex(const std::string& chunkSize) const;
};
