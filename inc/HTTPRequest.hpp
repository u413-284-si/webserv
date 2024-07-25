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
	statusCode errorCode;
	bool shallCloseConnection;
};
