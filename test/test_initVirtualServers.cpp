#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class InitVirtualServersTest : public ::testing::Test {
	protected:
	InitVirtualServersTest()
	{
		ServerConfig serverConfig;
		serverConfig.host = "127.0.0.1";
		serverConfig.port = 8080;

		ServerConfig serverConfig2;
		serverConfig2.host = "localhost";
		serverConfig2.port = 7070;

		configFile.serverConfigs.push_back(serverConfig);
		configFile.serverConfigs.push_back(serverConfig2);

		server = new Server(configFile, epollWrapper, socketPolicy);

		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}

	~InitVirtualServersTest() override
	{
		delete server;
	}

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server* server;

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
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct sockaddr* addr2 = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = {
		.ai_addr = addr,
		.ai_next = nullptr
	};
	*addrinfo2 = {
		.ai_addr = addr2,
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

	EXPECT_EQ(initVirtualServers(*server, backlog, server->getServerConfigs()), true);
	EXPECT_EQ(server->getVirtualServers().size(), 2);
	EXPECT_EQ(server->getVirtualServers().at(dummyFd).host, configFile.serverConfigs[0].host);
	EXPECT_EQ(server->getVirtualServers().at(dummyFd).port, webutils::toString(configFile.serverConfigs[0].port));
	EXPECT_EQ(server->getVirtualServers().at(dummyFd2).host, configFile.serverConfigs[1].host);
	EXPECT_EQ(server->getVirtualServers().at(dummyFd2).port, webutils::toString(configFile.serverConfigs[1].port));
}
