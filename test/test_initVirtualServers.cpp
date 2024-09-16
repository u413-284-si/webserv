#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class InitVirtualServersTest : public ::testing::Test {
protected:
	InitVirtualServersTest() : server(configFile, epollWrapper, socketPolicy)
	{
		ConfigServer serverConfig;
		serverConfig.host = "127.0.0.1";
		serverConfig.port = "8080";

		ConfigServer serverConfig2;
		serverConfig2.host = "localhost";
		serverConfig2.port = "7070";

		configFile.servers.push_back(serverConfig);
		configFile.servers.push_back(serverConfig2);

		ON_CALL(epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}

	~InitVirtualServersTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	const int backlog = 10;

	const int dummyFd = 10;
	Socket serverSock1 = { "127.0.0.1", "8080" };

	const int dummyFd2 = 11;
	Socket serverSock2 = { "localhost", "7070" };

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

	EXPECT_EQ(initVirtualServers(server, backlog, server.getServerConfigs()), true);
	EXPECT_EQ(server.getVirtualServers().size(), 2);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, configFile.servers[0].host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, configFile.servers[0].port);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd2).host, configFile.servers[1].host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd2).port, configFile.servers[1].port);
}

TEST_F(InitVirtualServersTest, OneDuplicateServer)
{
	configFile.servers[1] = configFile.servers[0];

	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	*addrinfo = {
		.ai_addr = addr,
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
		.Times(1)
		.WillOnce(Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
		.Times(1)
		.WillOnce(Return(dummyFd));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
		.Times(1)
		.WillOnce(Return(serverSock1));

	EXPECT_EQ(initVirtualServers(server, backlog, server.getServerConfigs()), true);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, configFile.servers[0].host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, configFile.servers[0].port);
}

TEST_F(InitVirtualServersTest, NoServers)
{
	configFile.servers.clear();

	EXPECT_EQ(initVirtualServers(server, backlog, server.getServerConfigs()), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}
