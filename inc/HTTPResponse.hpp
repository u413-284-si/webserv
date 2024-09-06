#pragma once

#include "StatusCode.hpp"
#include <string>
#include "ConfigFile.hpp"

struct HTTPResponse {
	statusCode status;
	std::string	targetResource;
	std::string body;
	std::vector<ConfigServer>::const_iterator server;
	std::vector<Location>::const_iterator location;
	Method method;
	bool isAutoindex;
};

std::ostream& operator<<(std::ostream& ostream, const HTTPResponse& httpResponse);
