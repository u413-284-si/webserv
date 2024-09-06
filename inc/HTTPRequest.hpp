#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
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
	Method method;
	URI uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	statusCode httpStatus;
	bool shallCloseConnection;
	bool hasBody;
	bool isChunked;
	std::vector<ConfigServer>::const_iterator activeServer;
	std::vector<Location>::const_iterator activeLocation;
};

std::ostream& operator<<(std::ostream& ostream, const HTTPRequest& httpRequest);
std::ostream& operator<<(std::ostream& ostream, const URI& uri);
