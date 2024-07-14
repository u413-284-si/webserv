#pragma once

#include "StatusCode.hpp"
#include <string>
#include "ConfigFile.hpp"

struct HTTPResponse {
	statusCode status; 
	std::string	targetResource;
	std::string body;
	std::vector<Location>::const_iterator location;
	std::string method;
	bool autoindex;
};
