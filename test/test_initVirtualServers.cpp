#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;

class InitVirtualServersTest : public ::testing::Test {
	protected:
	InitVirtualServersTest() {
		serverConfig.host = "127.0.0.1";
		serverConfig.port = 8080;
		serverConfig2.host = "localhost";
		serverConfig2.port = 7070;
		serverConfigs.push_back(serverConfig);
		serverConfigs.push_back(serverConfig2);
	}
	~InitVirtualServersTest() override { }

	ServerConfig serverConfig;
	ServerConfig serverConfig2;
	std::vector<ServerConfig> serverConfigs;
	MockSocketPolicy socketPolicy;
	MockEpollWrapper epollWrapper;
	std::map<int, Socket> virtualServers;
	const int backlog = 10;

	const int dummyFd = 10;
	Socket serverSock1 = {
		"127.0.0.1",
		"8080"
	};
	const int dummyFd2 = 11;
	Socket serverSock2 = {
		"localhost",
		"7070"
	};
};

TEST_F(InitVirtualServersTest, ServerInitSuccess)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = {
		.ai_next = nullptr
	};
	*addrinfo2 = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(2)
	.WillOnce(Return(addrinfo))
	.WillOnce(Return(addrinfo2));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(2)
	.WillOnce(Return(dummyFd))
	.WillOnce(Return(dummyFd2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(2)
	.WillOnce(Return(serverSock1))
	.WillOnce(Return(serverSock2));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(2)
	.WillRepeatedly(Return(true));

	EXPECT_EQ(initVirtualServers(serverConfigs, socketPolicy, epollWrapper, virtualServers, backlog), true);
	EXPECT_EQ(virtualServers.size(), 2);
	EXPECT_EQ(virtualServers[dummyFd].host, serverConfig.host);
	EXPECT_EQ(virtualServers[dummyFd].port, webutils::toString(serverConfig.port));
	EXPECT_EQ(virtualServers[dummyFd2].host, serverConfig2.host);
	EXPECT_EQ(virtualServers[dummyFd2].port, webutils::toString(serverConfig2.port));
}
