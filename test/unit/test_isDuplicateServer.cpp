#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockFileSystemPolicy.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class isDuplicateServerTest : public ::testing::Test {
	protected:
	isDuplicateServerTest()
	{
		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}
	~isDuplicateServerTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockFileSystemPolicy fileSystemPolicy;
	MockSocketPolicy socketPolicy;
	MockProcessOps processOps;
	Server server = Server(configFile, epollWrapper, fileSystemPolicy, socketPolicy, processOps);
};

TEST_F(isDuplicateServerTest, EmptyMap)
{
	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(server, host, port), false);
}

TEST_F(isDuplicateServerTest, NoDuplicate)
{
	server.registerVirtualServer(10, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(20, (Socket { "192.168.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(server, host, port), false);
}

TEST_F(isDuplicateServerTest, DuplicateServer)
{
	server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));

	std::string host = "127.0.0.1";
	std::string port = "8080";

	EXPECT_EQ(isDuplicateServer(server, host, port), true);
}

TEST_F(isDuplicateServerTest, DuplicateServerLocalhost)
{
	server.registerVirtualServer(10, (Socket { "127.0.0.1", "6060" }));
	server.registerVirtualServer(20, (Socket { "127.0.0.1", "7070" }));
	server.registerVirtualServer(30, (Socket { "127.0.0.1", "8080" }));
	server.registerVirtualServer(40, (Socket { "::1", "9090" }));
	server.registerVirtualServer(50, (Socket { "::1", "1010" }));

	std::string host = "localhost";
	std::string port = "7070";
	std::string port2 = "2020";
	std::string port3 = "1010";

	EXPECT_EQ(isDuplicateServer(server, host, port), true);
	EXPECT_EQ(isDuplicateServer(server, host, port2), false);
	EXPECT_EQ(isDuplicateServer(server, host, port3), true);
}
