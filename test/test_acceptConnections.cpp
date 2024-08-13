#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockSocketPolicy.hpp"
#include "MockEpollWrapper.hpp"
#include "Server.hpp"

using ::testing::Return;

class AcceptConnectionsTest : public ::testing::Test {
	protected:
	AcceptConnectionsTest() : server(configFile, epollWrapper, socketPolicy) { }
	~AcceptConnectionsTest() override { }

	ConfigFile configFile;
	MockEpollWrapper epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	const int dummyServerFd = 10;
	Socket serverSock = {
		"127.0.0.1",
		"8080" };

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

	EXPECT_CALL(socketPolicy, acceptConnection)
	.Times(2)
	.WillOnce(Return(dummyClientFd))
	.WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(1)
	.WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 1);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getClientSocket().host, host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getClientSocket().port, port);

	// destructor calls removeEvent
	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(1);
}

TEST_F(AcceptConnectionsTest, AcceptThreeConnections)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptConnection)
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

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(3)
	.WillRepeatedly(Return(true));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 3);

	EXPECT_EQ(server.getConnections().at(dummyClientFd).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getClientSocket().host, host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd).getClientSocket().port, port);

	EXPECT_EQ(server.getConnections().at(dummyClientFd2).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).getClientSocket().host, host2);
	EXPECT_EQ(server.getConnections().at(dummyClientFd2).getClientSocket().port, port2);

	EXPECT_EQ(server.getConnections().at(dummyClientFd3).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).getClientSocket().host, host3);
	EXPECT_EQ(server.getConnections().at(dummyClientFd3).getClientSocket().port, port3);

	// destructor calls removeEvent
	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(3);
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

	EXPECT_CALL(socketPolicy, acceptConnection)
	.Times(2)
	.WillOnce(Return(-1))
	.WillOnce(Return(-2));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}


TEST_F(AcceptConnectionsTest, retrieveSocketInfoFail)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptConnection)
	.Times(2)
	.WillOnce(Return(dummyClientFd))
	.WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(1).
	WillOnce(Return(Socket { "", "" }));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(AcceptConnectionsTest, registerConnectionFail)
{
	uint32_t eventMask = EPOLLIN;

	EXPECT_CALL(socketPolicy, acceptConnection)
	.Times(2)
	.WillOnce(Return(dummyClientFd))
	.WillOnce(Return(-2));

	EXPECT_CALL(socketPolicy, retrieveSocketInfo)
	.Times(1)
	.WillOnce(Return(Socket { host, port }));

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	acceptConnections(server, dummyServerFd, serverSock, eventMask);

	EXPECT_EQ(server.getConnections().size(), 0);
}
