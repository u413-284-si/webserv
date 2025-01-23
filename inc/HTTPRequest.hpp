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

	Method method;
	URI uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string boundary;
	long chunkSize;
	unsigned long contentLength;
	statusCode httpStatus;
	bool shallCloseConnection;
	bool hasBody;
	bool isCompleteBody;
	bool isChunked;
	std::string targetResource;
	bool isDirectory;
	bool hasAutoindex;
	bool hasCGI;
	bool hasReturn;
	bool hasMultipartFormdata;
};

std::ostream& operator<<(std::ostream& ostream, const HTTPRequest& httpRequest);
std::ostream& operator<<(std::ostream& ostream, const URI& uri);
