#pragma once

#include "StatusCode.hpp"
#include <string>
#include "ConfigFile.hpp"

struct HTTPResponse {
	statusCode status;
	std::string	targetResource;
	std::string response;
	std::vector<Location>::const_iterator location;
};
