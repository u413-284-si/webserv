#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class CreateVirtualServerTest : public ::testing::Test {
	protected:
	CreateVirtualServerTest() : server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}
	~CreateVirtualServerTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	std::string host = "127.0.0.1";
	std::string port = "8080";
	const int backlog = 10;
	const int dummyFd = 10;
	const int dummyFd2 = 11;
	const int dummyFd3 = 12;
};

TEST_F(CreateVirtualServerTest, ServerAddSuccess)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = {
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
	.WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), true);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, port);
}

TEST_F(CreateVirtualServerTest, resolveListeningAddressesFails)
{
	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(nullptr));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}

TEST_F(CreateVirtualServerTest, createListeningSocketFails)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(1)
	.WillOnce(Return(-1));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}

TEST_F(CreateVirtualServerTest, retrieveSocketInfoFails)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = {
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
	.WillOnce(Return(Socket { "", "" }));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}

TEST_F(CreateVirtualServerTest, registerVirtualServerFails)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	*addrinfo = {
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
	.WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), false);
	EXPECT_EQ(server.getVirtualServers().size(), 0);
}

TEST_F(CreateVirtualServerTest, FirstFailsSecondSuccess)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = {
		.ai_next = addrinfo2
	};
	*addrinfo2 = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(2)
	.WillOnce(Return(dummyFd))
	.WillOnce(Return(-1));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(1)
	.WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), true);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, port);
}

TEST_F(CreateVirtualServerTest, FirstSuccessSecondFail)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	*addrinfo = {
		.ai_next = addrinfo2
	};
	*addrinfo2 = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(2)
	.WillOnce(Return(dummyFd))
	.WillOnce(Return(dummyFd));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(2)
	.WillOnce(Return(Socket { "", "" }))
	.WillOnce(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), true);
	EXPECT_EQ(server.getVirtualServers().size(), 1);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, port);
}

TEST_F(CreateVirtualServerTest, AddThree)
{
	struct addrinfo* addrinfo = (struct addrinfo*)malloc(sizeof(*addrinfo));
	struct addrinfo* addrinfo2 = (struct addrinfo*)malloc(sizeof(*addrinfo2));
	struct addrinfo* addrinfo3 = (struct addrinfo*)malloc(sizeof(*addrinfo3));
	*addrinfo = {
		.ai_next = addrinfo2
	};
	*addrinfo2 = {
		.ai_next = addrinfo3
	};
	*addrinfo3 = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(3)
	.WillOnce(Return(dummyFd))
	.WillOnce(Return(dummyFd2))
	.WillOnce(Return(dummyFd3));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(3)
	.WillRepeatedly(Return(Socket { host, port }));

	EXPECT_EQ(createVirtualServer(server, host, backlog, port), true);
	EXPECT_EQ(server.getVirtualServers().size(), 3);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd).port, port);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd2).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd2).port, port);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd3).host, host);
	EXPECT_EQ(server.getVirtualServers().at(dummyFd3).port, port);
}
