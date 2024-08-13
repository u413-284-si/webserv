#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;

class CheckDuplicateServerTest : public ::testing::Test {
	protected:
	CheckDuplicateServerTest() : server(configFile, epollWrapper, socketPolicy) { }
	~CheckDuplicateServerTest() override { }

	ConfigFile configFile;
	MockEpollWrapper epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
};

TEST_F(CheckDuplicateServerTest, EmptyMap)
{
	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(server, host, port), false);
}

TEST_F(CheckDuplicateServerTest, NoDuplicate)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(2)
	.WillRepeatedly(Return(true));

	server.registerVirtualServer(10, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(20, (Socket { "192.168.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(server, host, port), false);
}

TEST_F(CheckDuplicateServerTest, DuplicateServer)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(3)
	.WillRepeatedly(Return(true));

	server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(checkDuplicateServer(server, host, port), true);
}

TEST_F(CheckDuplicateServerTest, DuplicateServerLocalhost)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(5)
	.WillRepeatedly(Return(true));

	server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));
	server.registerVirtualServer(40, (Socket { "::1", "9090" }));
	server.registerVirtualServer(50, (Socket { "::1", "1010" }));

	std::string host = "localhost";
	std::string port = "7070";
	std::string port2 = "2020";
	std::string port3 = "1010";

	EXPECT_EQ(checkDuplicateServer(server, host, port), true);
	EXPECT_EQ(checkDuplicateServer(server, host, port2), false);
	EXPECT_EQ(checkDuplicateServer(server, host, port3), true);
}
