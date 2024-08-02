#pragma once

#include <string>

struct Socket {
	int fd;
	std::string host;
	std::string port;
};

std::ostream& operator<<(std::ostream& ostream, const Socket& socket);
