#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "Server.hpp"

using ::testing::Return;

class RegisterConnectionTest : public ::testing::Test {
	protected:
	RegisterConnectionTest() { }
	~RegisterConnectionTest() override { }

	MockEpollWrapper epollWrapper;
	std::map<int, Connection> connections;
	std::map<int, std::string> connectionBuffers;
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

	EXPECT_EQ(registerConnection(serverSock, dummyFd, clientSocket, connections, connectionBuffers, epollWrapper), true);
	EXPECT_EQ(connections.size(), 1);
	EXPECT_EQ(connections[dummyFd].getServerSocket().host, serverSock.host);
	EXPECT_EQ(connections[dummyFd].getServerSocket().port, serverSock.port);
	EXPECT_EQ(connections[dummyFd].getClientSocket().host, clientSocket.host);
	EXPECT_EQ(connections[dummyFd].getClientSocket().port, clientSocket.port);
}

TEST_F(RegisterConnectionTest, ConnectionRegisterFail)
{
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(registerConnection(serverSock, dummyFd, clientSocket, connections, connectionBuffers, epollWrapper), false);
	EXPECT_EQ(connections.size(), 0);
}

TEST_F(RegisterConnectionTest, OverwriteOldConnection)
{
	Socket oldServerSock = {
		"0.0.0.0",
		"0" };
	Socket oldClientSocket = {
		"1.1.1.1",
		"11111" };
	Connection oldConnection(oldServerSock, oldClientSocket);
	connections[dummyFd] = oldConnection;
	connectionBuffers[dummyFd] = "Old connection buffer";

	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillRepeatedly(Return(true));

	EXPECT_EQ(registerConnection(serverSock, dummyFd, clientSocket, connections, connectionBuffers, epollWrapper), true);
	EXPECT_EQ(connections.size(), 1);
	EXPECT_EQ(connections[dummyFd].getServerSocket().host, serverSock.host);
	EXPECT_EQ(connections[dummyFd].getServerSocket().port, serverSock.port);
	EXPECT_EQ(connections[dummyFd].getClientSocket().host, clientSocket.host);
	EXPECT_EQ(connections[dummyFd].getClientSocket().port, clientSocket.port);
	EXPECT_EQ(connectionBuffers[dummyFd], "");
}
