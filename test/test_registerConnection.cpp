#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;

class RegisterConnectionTest : public ::testing::Test {
	protected:
	RegisterConnectionTest() :server(configFile, epollWrapper, socketPolicy) { }
	~RegisterConnectionTest() override { }

	ConfigFile configFile;
	MockEpollWrapper epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	Socket serverSock = {
		"127.0.0.1",
		"8080" };
	const int dummyFd = 10;
	Socket clientSocket = {
		"192.168.0.1",
		"12345" };
};

TEST_F(RegisterConnectionTest, ConnectionRegisterSuccess)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(true));

	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);
	EXPECT_EQ(server.getConnections().at(dummyFd).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyFd).getClientSocket().host, clientSocket.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).getClientSocket().port, clientSocket.port);

	// destructor calls removeEvent
	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(1);
}

TEST_F(RegisterConnectionTest, ConnectionRegisterFail)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), false);
	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(RegisterConnectionTest, OverwriteOldConnection)
{
	Socket oldServerSock = {
		"0.0.0.0",
		"0" };
	Socket oldClientSocket = {
		"1.1.1.1",
		"11111" };

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(2)
	.WillRepeatedly(Return(true));

	EXPECT_EQ(server.registerConnection(oldServerSock, dummyFd, oldClientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);

	// reuse the same dummyFd
	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);

	EXPECT_EQ(server.getConnections().at(dummyFd).getServerSocket().host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).getServerSocket().port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyFd).getClientSocket().host, clientSocket.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).getClientSocket().port, clientSocket.port);

	// destructor calls removeEvent
	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(1);
}
