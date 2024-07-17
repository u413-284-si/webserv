#pragma once

/* ====== LIBRARIES ====== */

#include <iostream>
#include <map>
#include <string>

/* ====== DEFINITIONS ====== */

enum Method { MethodGet = 0, MethodPost = 1, MethodDelete = 2, MethodCount = 3 };

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
	unsigned int errorCode;
	bool shallCloseConnection;
};
