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
		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));
		serverConfig.host = "1.1.1.1";
		serverConfig.port = "8080";
		configFile.servers.push_back(serverConfig);
	}
	~CheckForTimeoutTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	ConfigServer serverConfig;

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;

	const Socket dummySocket = { "1.1.1.1", "8080" };
};

TEST_F(CheckForTimeoutTest, Timeout)
{
	server.setClientTimeout(-1);

	server.registerConnection(dummySocket, dummyFd, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::Timeout);
}

TEST_F(CheckForTimeoutTest, NoTimeout)
{
	server.setClientTimeout(100);

	server.registerConnection(dummySocket, dummyFd, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::ReceiveHeader);
}

TEST_F(CheckForTimeoutTest, MultipleTimeouts)
{
	server.setClientTimeout(-1);

	server.registerConnection(dummySocket, dummyFd, dummySocket);
	server.registerConnection(dummySocket, dummyFd2, dummySocket);
	server.registerConnection(dummySocket, dummyFd3, dummySocket);

	checkForTimeout(server);

	EXPECT_EQ(server.getConnections().at(dummyFd).m_status, Connection::Timeout);
	EXPECT_EQ(server.getConnections().at(dummyFd2).m_status, Connection::Timeout);
	EXPECT_EQ(server.getConnections().at(dummyFd3).m_status, Connection::Timeout);
}
