#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class RegisterConnectionTest : public ::testing::Test {
	protected:
	RegisterConnectionTest() :server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
		serverConfig.host = "127.0.0.1";
		serverConfig.port = "8080";
		serverConfig2.host = "0.0.0.0";
		serverConfig2.port = "0";
		configFile.servers.push_back(serverConfig);
		configFile.servers.push_back(serverConfig2);
	}
	~RegisterConnectionTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	ConfigServer serverConfig;
	ConfigServer serverConfig2;

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
	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);
	EXPECT_EQ(server.getConnections().at(dummyFd).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getConnections().at(dummyFd).m_clientSocket.host, clientSocket.host);
	EXPECT_EQ(server.getConnections().at(dummyFd).m_clientSocket.port, clientSocket.port);
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

	EXPECT_EQ(server.registerConnection(oldServerSock, dummyFd, oldClientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);

	// reuse the same dummyFd should fail as a connection with this fd already exists
	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), false);
	EXPECT_EQ(server.getConnections().size(), 1);
}
