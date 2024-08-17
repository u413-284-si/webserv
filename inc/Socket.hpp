#pragma once

#include <string>

/**
 * @brief Represent a socket connection.
 */
struct Socket {
	std::string host;
	std::string port;
};

std::ostream& operator<<(std::ostream& ostream, const Socket& socket);
