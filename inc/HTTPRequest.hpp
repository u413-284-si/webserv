#pragma once

/* ====== LIBRARIES ====== */

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
	std::string method;
	URI uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	bool hasBody;
	bool chunked;
};
