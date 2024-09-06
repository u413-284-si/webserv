#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class CheckForTimeoutTest : public ::testing::Test {
protected:
	CheckForTimeoutTest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		ConfigServer serverConfig;
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);

		ON_CALL(epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}
	~CheckForTimeoutTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;

	const Socket serverSock = { "127.0.0.1", "8080" };
	const Socket dummySocket = { "1.1.1.1", "8080" };
};

TEST_F(CheckForTimeoutTest, Timeout)
{
	server.setClientTimeout(-1);

	server.registerConnection(serverSock, dummyFd, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::Timeout);
}

TEST_F(CheckForTimeoutTest, NoTimeout)
{
	server.setClientTimeout(100);

	server.registerConnection(serverSock, dummyFd, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::ReceiveHeader);
}

TEST_F(CheckForTimeoutTest, MultipleTimeouts)
{
	server.setClientTimeout(-1);

	server.registerConnection(serverSock, dummyFd, dummySocket);
	server.registerConnection(serverSock, dummyFd2, dummySocket);
	server.registerConnection(serverSock, dummyFd3, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::Timeout);
	EXPECT_EQ(server.getConnections().at(dummyFd2).m_status, Connection::Timeout);
	EXPECT_EQ(server.getConnections().at(dummyFd3).m_status, Connection::Timeout);
}
