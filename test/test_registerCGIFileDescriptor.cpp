#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class RegisterCGITest : public ::testing::Test {
	protected:
	RegisterCGITest() 
		: server(configFile, epollWrapper, socketPolicy)
	{
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);

		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}
	~RegisterCGITest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	ConfigServer serverConfig;
	const int dummyFd = 10;

	Socket serverSock = {
		"127.0.0.1",
		"8080" };
	Socket clientSocket = {
		"192.168.0.1",
		"12345" };
};

TEST_F(RegisterCGITest, CGIRegisterSuccess)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);

	// Act & Assert
	EXPECT_EQ(server.registerCGIFileDescriptor(dummyFd, EPOLLIN, connection), true);
	EXPECT_EQ(server.getCGIConnections().size(), 1);
	EXPECT_EQ(server.getCGIConnections().at(dummyFd)->m_serverSocket.host, serverSock.host);
	EXPECT_EQ(server.getCGIConnections().at(dummyFd)->m_serverSocket.port, serverSock.port);
	EXPECT_EQ(server.getCGIConnections().at(dummyFd)->m_clientSocket.host, clientSocket.host);
	EXPECT_EQ(server.getCGIConnections().at(dummyFd)->m_clientSocket.port, clientSocket.port);
}

TEST_F(RegisterCGITest, CGIRegisterFail)
{
	// Arrange
	EXPECT_CALL(epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);

	// Act & Assert
	EXPECT_EQ(server.registerCGIFileDescriptor(dummyFd, EPOLLIN, connection), false);
	EXPECT_EQ(server.getCGIConnections().size(), 0);
}
