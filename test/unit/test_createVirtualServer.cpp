#include "test_helpers.hpp"
#include <cstdlib>
#include <sys/socket.h>

using ::testing::Return;

class CreateVirtualServerTest : public ServerTestBase {
protected:
	CreateVirtualServerTest() { ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true)); }
	~CreateVirtualServerTest() override { }

	std::string host = "127.0.0.1";
	std::string port = "8080";
	const int backlog = 10;
	const int dummyFd = 10;
	const int dummyFd2 = 11;
	const int dummyFd3 = 12;
};

TEST_F(CreateVirtualServerTest, ServerAddSuccess)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(1).WillOnce(Return(dummyFd));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, port);

	free(addr);
}

TEST_F(CreateVirtualServerTest, resolveListeningAddressesFails)
{
	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(nullptr));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);
}

TEST_F(CreateVirtualServerTest, createListeningSocketFails)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(1).WillOnce(Return(-1));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);

	free(addr);
}

TEST_F(CreateVirtualServerTest, retrieveSocketInfoFails)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(1).WillOnce(Return(dummyFd));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { "", "" }));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);

	free(addr);
}

TEST_F(CreateVirtualServerTest, registerVirtualServerFails)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = { .ai_addr = addr, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(1).WillOnce(Return(dummyFd));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(m_epollWrapper, addEvent).Times(1).WillOnce(Return(false));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), false);
	EXPECT_EQ(m_server.getVirtualServers().size(), 0);

	free(addr);
}

TEST_F(CreateVirtualServerTest, FirstFailsSecondSuccess)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct sockaddr* addr2 = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = { .ai_addr = addr, .ai_next = addrinfo2 };
	*addrinfo2 = { .ai_addr = addr2, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(2).WillOnce(Return(dummyFd)).WillOnce(Return(-1));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, port);

	free(addr);
	free(addr2);
}

TEST_F(CreateVirtualServerTest, FirstSuccessSecondFail)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct sockaddr* addr2 = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = { .ai_addr = addr, .ai_next = addrinfo2 };
	*addrinfo2 = { .ai_addr = addr2, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket).Times(2).WillOnce(Return(dummyFd)).WillOnce(Return(dummyFd));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo)
		.Times(2)
		.WillOnce(Return(Socket { "", "" }))
		.WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, port);

	free(addr);
	free(addr2);
}

TEST_F(CreateVirtualServerTest, AddThree)
{
	struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(*addr));
	struct sockaddr* addr2 = (struct sockaddr*)malloc(sizeof(*addr));
	struct sockaddr* addr3 = (struct sockaddr*)malloc(sizeof(*addr));
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	struct addrinfo* addrinfo3 = (struct addrinfo*)malloc(sizeof(*addrinfo3));
	*addrinfo = { .ai_addr = addr, .ai_next = addrinfo2 };
	*addrinfo2 = { .ai_addr = addr2, .ai_next = addrinfo3 };
	*addrinfo3 = { .ai_addr = addr3, .ai_next = nullptr };

	EXPECT_CALL(m_socketOps, resolveListeningAddresses).Times(1).WillOnce(Return(addrinfo));

	EXPECT_CALL(m_socketOps, createListeningSocket)
		.Times(3)
		.WillOnce(Return(dummyFd))
		.WillOnce(Return(dummyFd2))
		.WillOnce(Return(dummyFd3));

	EXPECT_CALL(m_socketOps, retrieveSocketInfo).Times(3).WillRepeatedly(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(m_server, host, backlog, port), true);
	EXPECT_EQ(m_server.getVirtualServers().size(), 3);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd).port, port);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd2).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd2).port, port);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd3).host, host);
	EXPECT_EQ(m_server.getVirtualServers().at(dummyFd3).port, port);

	free(addr);
	free(addr2);
	free(addr3);
}
