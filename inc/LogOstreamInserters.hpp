#pragma once

#include <iostream>

#include "ConfigFile.hpp"
#include "StatusCode.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

// ConfigFile.hpp
std::ostream& operator<<(std::ostream& ostream, const Location& location);
std::ostream& operator<<(std::ostream& ostream, const ServerConfig& serverConfig);
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile);
std::ostream& operator<<(std::ostream& ostream, Method method);

// StatusCode.hpp
std::ostream& operator<<(std::ostream& ostream, statusCode statusCode);

// HTTPRequest.hpp
std::ostream& operator<<(std::ostream& ostream, const HTTPRequest& httpRequest);
std::ostream& operator<<(std::ostream& ostream, const URI& uri);

// HTTPResponse.hpp
std::ostream& operator<<(std::ostream& ostream, const HTTPResponse& httpResponse);
