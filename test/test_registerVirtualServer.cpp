#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "Server.hpp"

using ::testing::Return;

class RegisterVirtualServerTest : public ::testing::Test {
	protected:
	RegisterVirtualServerTest() { }
	~RegisterVirtualServerTest() override { }

	std::map<int, Socket> virtualServers;
	MockEpollWrapper epollWrapper;
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

	EXPECT_EQ(registerVirtualServer(dummyFd, socket, virtualServers, epollWrapper), true);
	EXPECT_EQ(virtualServers.size(), 1);
	EXPECT_EQ(virtualServers[dummyFd].host, socket.host);
	EXPECT_EQ(virtualServers[dummyFd].port, socket.port);
}

TEST_F(RegisterVirtualServerTest, ServerRegisterFail)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(registerVirtualServer(dummyFd, socket, virtualServers, epollWrapper), false);
	EXPECT_EQ(virtualServers.size(), 0);
}

TEST_F(RegisterVirtualServerTest, ServerAlreadyPresent)
{
	virtualServers[dummyFd] = socket2;

	// epoll will fail to add the event because the fd is already present
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(registerVirtualServer(dummyFd, socket, virtualServers, epollWrapper), false);
	EXPECT_EQ(virtualServers.size(), 1);
	EXPECT_EQ(virtualServers[dummyFd].host, socket2.host);
	EXPECT_EQ(virtualServers[dummyFd].port, socket2.port);
}
