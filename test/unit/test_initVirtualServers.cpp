#include "test_helpers.hpp"

using ::testing::Return;

class InitVirtualServersTest : public ServerTestBase {
protected:
	InitVirtualServersTest()
	{
		ConfigServer serverConfig2;
		serverConfig2.host = "localhost";
		serverConfig2.port = "7070";

		m_configFile.servers.push_back(serverConfig2);

		ON_CALL(m_epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}

	~InitVirtualServersTest() override { }

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
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };
	*addrinfo2 = { .ai_addr = addr2, .ai_next = nullptr };

	EXPECT_CALL(m_socketPolicy, resolveListeningAddresses)
		.Times(2)
		.WillOnce(Return(addrinfo))
		.WillOnce(Return(addrinfo2));

	EXPECT_CALL(m_socketPolicy, createListeningSocket).Times(2).WillOnce(Return(dummyFd)).WillOnce(Return(dummyFd2));

	EXPECT_CALL(m_socketPolicy, retrieveSocketInfo).Times(2).WillOnce(Return(serverSock1)).WillOnce(Return(serverSock2));

	EXPECT_EQ(initVirtualServers(m_server, backlog, m_server.getServerConfigs()), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 2);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, m_configFile.servers[0].host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, m_configFile.servers[0].port);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd2).host, m_configFile.servers[1].host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd2).port, m_configFile.servers[1].port);

	free(addr);
	free(addr2);
}

TEST_F(InitVirtualServersTest, OneDuplicateServer)
{
	m_configFile.servers[1] = m_configFile.servers[0];

	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };

	EXPECT_CALL(m_socketPolicy, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketPolicy, createListeningSocket).Times(1).WillOnce(Return(dummyFd));

	EXPECT_CALL(m_socketPolicy, retrieveSocketInfo).Times(1).WillOnce(Return(serverSock1));

	EXPECT_EQ(initVirtualServers(m_server, backlog, m_server.getServerConfigs()), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, m_configFile.servers[0].host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, m_configFile.servers[0].port);

	free(addr);
}

TEST_F(InitVirtualServersTest, NoServers)
{
	m_configFile.servers.clear();

	EXPECT_EQ(initVirtualServers(m_server, backlog, m_server.getServerConfigs()), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);
}
