#include <gtest/gtest.h>

#include "Server.hpp"

TEST(checkDuplicateServer, EmptyMap)
{
	std::map<int, Socket> virtualServers;

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), false);
}

TEST(checkDuplicateServer, NoDuplicate)
{
	std::map<int, Socket> virtualServers;
	virtualServers[1] = Socket{ 1, "127.0.0.1", "7070" };
	virtualServers[2] = Socket{ 2, "192.168.0.1", "8080" };

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), false);
}

TEST(checkDuplicateServer, DuplicateServer)
{
	std::map<int, Socket> virtualServers;
	virtualServers[1] = Socket{ 1, "127.0.0.1", "6060" };
	virtualServers[2] = Socket{ 2, "127.0.0.1", "7070" };
	virtualServers[3] = Socket{ 3, "127.0.0.1", "8080" };

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), true);
}

TEST(checkDuplicateServer, DuplicateServerLocalhost)
{
	std::map<int, Socket> virtualServers;
	virtualServers[1] = Socket{ 1, "127.0.0.1", "6060" };
	virtualServers[2] = Socket{ 2, "127.0.0.1", "7070" };
	virtualServers[3] = Socket{ 3, "127.0.0.1", "8080" };
	virtualServers[4] = Socket{ 4, "::1", "9090" };
	virtualServers[5] = Socket{ 5, "::1", "1010" };

	std::string host = "localhost";
	std::string port = "7070";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), true);

	port = "2020";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), false);

	port = "1010";

	EXPECT_EQ(checkDuplicateServer(virtualServers, host, port), true);
}