#pragma once

#include "StatusCode.hpp"
#include <string>

struct HTTPResponse {
	statusCode status;
	std::string	targetResource;
	std::string response;
};
