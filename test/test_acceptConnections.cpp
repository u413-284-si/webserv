#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class AcceptConnectionsTest : public ::testing::Test {
protected:
	AcceptConnectionsTest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));
	}
	~AcceptConnectionsTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	const int dummyServerFd = 10;
	Socket serverSock = { "127.0.0.1", "8080" };

	const int dummyClientFd = 11;
	std::string host = "1.1.1.1";
	std::string port = "11111";

	const int dummyClientFd2 = 12;
	std::string host2 = "2.2.2.2";
	std::string port2 = "22222";

	const int dummyClientFd3 = 13;
	std::string host3 = "3.3.3.3";
	std::string port3 = "33333";
};

TEST_F(AcceptConnectionsTest, AcceptConnectionsSuccess)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptSingleConnection).Times(2).WillOnce(Return(dummyClientFd)).WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { host, port }));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 1);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_clientSocket.host, host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_clientSocket.port, port);
}

TEST_F(AcceptConnectionsTest, AcceptThreeConnections)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptSingleConnection)
		.Times(4)
		.WillOnce(Return(dummyClientFd))
		.WillOnce(Return(dummyClientFd2))
		.WillOnce(Return(dummyClientFd3))
		.WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
		.Times(3)
		.WillOnce(Return(Socket { host, port }))
		.WillOnce(Return(Socket { host2, port2 }))
		.WillOnce(Return(Socket { host3, port3 }));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 3);

	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_clientSocket.host, host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).m_clientSocket.port, port);

	EXPECT_EQ(server.getConnections().at(dummyClientFd2).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).m_clientSocket.host, host2);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).m_clientSocket.port, port2);

	EXPECT_EQ(server.getConnections().at(dummyClientFd3).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).m_clientSocket.host, host3);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).m_clientSocket.port, port3);
}

TEST_F(AcceptConnectionsTest, UnkownEvent)
{
	uint32_t eventMask = EPOLLOUT;

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(AcceptConnectionsTest, acceptConnectionFail)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptSingleConnection).Times(2).WillOnce(Return(-1)).WillOnce(Return(-2));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(AcceptConnectionsTest, retrieveSocketInfoFail)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptSingleConnection).Times(2).WillOnce(Return(dummyClientFd)).WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { "", "" }));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(AcceptConnectionsTest, registerConnectionFail)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptSingleConnection).Times(2).WillOnce(Return(dummyClientFd)).WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo).Times(1).WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent).Times(1).WillOnce(Return(false));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}
