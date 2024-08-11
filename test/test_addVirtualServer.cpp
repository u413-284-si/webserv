#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

class AddVirtualServerTest : public ::testing::Test {
	protected:
	AddVirtualServerTest() { }
	~AddVirtualServerTest() override { }

	std::map<int, Socket> virtualServers;
	MockSocketPolicy socketPolicy;
	MockEpollWrapper epollWrapper;
};

TEST(addVirtualServer, Success)
{
	std::map<int, Socket> virtualServers;
	MockSocketPolicy socketPolicy;
	MockEpollWrapper epollWrapper;
	std::string host = "127.0.0.1";
	std::string port = "8080";
	const int backlog = 10;
	struct addrinfo* addrinfo = new struct addrinfo;
	*addrinfo = {
		.ai_next = nullptr
	};

	EXPECT_CALL(socketPolicy, resolveListeningAddresses)
	.Times(1)
	.WillOnce(testing::Return(addrinfo));

	EXPECT_CALL(socketPolicy, createListeningSocket)
	.Times(1)
	.WillOnce(testing::Return(1));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(1).
	WillOnce(testing::Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(testing::Return(true));

	EXPECT_EQ(addVirtualServer(socketPolicy, epollWrapper, virtualServers, host, backlog, port), true);
}
