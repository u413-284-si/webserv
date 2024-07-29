#pragma once

#include <string>

struct Socket {
	int fd;
	std::string host;
	std::string port;
};


struct Connection {
	Socket clientSock;
	Socket serverSock;
};
