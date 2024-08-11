#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;

class AddVirtualServerTest : public ::testing::Test {
	protected:
	AddVirtualServerTest() { }
	~AddVirtualServerTest() override { }

	std::map<int, Socket> virtualServers;
	MockSocketPolicy socketPolicy;
	MockEpollWrapper epollWrapper;
	std::string host = "127.0.0.1";
	std::string port = "8080";
	const int backlog = 10;
	const int dummyFd = 10;
};

TEST_F(AddVirtualServerTest, ServerAddSuccess)
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
	.Times(1).
	WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), true);
	EXPECT_EQ(virtualServers.size(), 1);
	EXPECT_EQ(virtualServers[dummyFd].host, host);
	EXPECT_EQ(virtualServers[dummyFd].port, port);
}

TEST_F(AddVirtualServerTest, resolveListeningAddressesFails)
{
	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(Return(nullptr));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), false);
	EXPECT_EQ(virtualServers.size(), 0);
}

TEST_F(AddVirtualServerTest, createListeningSocketFails)
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

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), false);
	EXPECT_EQ(virtualServers.size(), 0);
}

TEST_F(AddVirtualServerTest, retrieveSocketInfoFails)
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
	.Times(1).
	WillOnce(Return(Socket { "", "" }));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), false);
	EXPECT_EQ(virtualServers.size(), 0);
}

TEST_F(AddVirtualServerTest, registerVirtualServerFails)
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
	.Times(1).
	WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), false);
	EXPECT_EQ(virtualServers.size(), 0);
}

TEST_F(AddVirtualServerTest, FirstFailsSecondSuccess)
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
	.Times(1).
	WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), true);
	EXPECT_EQ(virtualServers.size(), 1);
	EXPECT_EQ(virtualServers[dummyFd].host, host);
	EXPECT_EQ(virtualServers[dummyFd].port, port);
}

TEST_F(AddVirtualServerTest, FirstSuccessSecondFail)
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

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), true);
	EXPECT_EQ(virtualServers.size(), 1);
	EXPECT_EQ(virtualServers[dummyFd].host, host);
	EXPECT_EQ(virtualServers[dummyFd].port, port);
}

TEST_F(AddVirtualServerTest, AddThree)
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
	.WillOnce(Return(dummyFd + 1))
	.WillOnce(Return(dummyFd + 2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(3)
	.WillRepeatedly(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(3)
	.WillRepeatedly(Return(true));

	EXPECT_EQ(addVirtualServer(host, backlog, port, virtualServers, epollWrapper, socketPolicy), true);
	EXPECT_EQ(virtualServers.size(), 3);
	EXPECT_EQ(virtualServers[dummyFd].host, host);
	EXPECT_EQ(virtualServers[dummyFd].port, port);
	EXPECT_EQ(virtualServers[dummyFd + 1].host, host);
	EXPECT_EQ(virtualServers[dummyFd + 1].port, port);
	EXPECT_EQ(virtualServers[dummyFd + 2].host, host);
	EXPECT_EQ(virtualServers[dummyFd + 2].port, port);
}
