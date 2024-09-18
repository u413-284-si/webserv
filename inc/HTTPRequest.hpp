#pragma once

/* ====== LIBRARIES ====== */

#include "Method.hpp"
#include "StatusCode.hpp"
#include <iostream>
#include <map>
#include <string>

/* ====== DEFINITIONS ====== */

struct URI {
	std::string path;
	std::string query;
	std::string fragment;
};

struct HTTPRequest {
	HTTPRequest();

	HTTPRequest();

	Method method;
	URI uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	statusCode httpStatus;
	bool shallCloseConnection;
	bool hasBody;
	bool isChunked;
	std::string targetResource;
	bool hasAutoindex;
	bool isCGI;
};

std::ostream& operator<<(std::ostream& ostream, const HTTPRequest& httpRequest);
std::ostream& operator<<(std::ostream& ostream, const URI& uri);
