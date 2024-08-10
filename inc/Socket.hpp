#pragma once

#include <string>

/**
 * @brief Represent a socket connection.
 */
struct Socket {
	std::string host;
	std::string port;
};

/**
 * @brief Output the socket information to the output stream.
 *
 * @param ostream Output stream
 * @param socket Socket to output
 * @return std::ostream& Output stream
 */
std::ostream& operator<<(std::ostream& ostream, const Socket& socket);
