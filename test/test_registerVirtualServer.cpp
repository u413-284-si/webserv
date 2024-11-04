#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class RegisterVirtualServerTest : public ::testing::Test {
	protected:
	RegisterVirtualServerTest() : server(configFile, epollWrapper, socketPolicy, processOps) { }
	~RegisterVirtualServerTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
    MockProcessOps processOps;
	Server server;

	Socket socket = {
		"127.0.0.1",
		"8080" };
	Socket socket2 = {
		"192.168.0.1",
		"7070" };
	const int dummyFd = 10;
};

TEST_F(RegisterVirtualServerTest, ServerRegisterSuccess)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(server.registerVirtualServer(dummyFd, socket), true);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, socket.host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, socket.port);
}

TEST_F(RegisterVirtualServerTest, ServerRegisterFail)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(server.registerVirtualServer(dummyFd, socket), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}

TEST_F(RegisterVirtualServerTest, ServerAlreadyPresent)
{
	// epoll will fail the second time to add the event because the fd is already present
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(2)
	.WillOnce(Return(true))
	.WillOnce(Return(false));

	EXPECT_EQ(server.registerVirtualServer(dummyFd, socket2), true);

	EXPECT_EQ(server.registerVirtualServer(dummyFd, socket), false);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, socket2.host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, socket2.port);
}
