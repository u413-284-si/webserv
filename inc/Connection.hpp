#pragma once

#include <string>

struct Connection {
	int fd;
	std::string host;
	std::string port;
};
