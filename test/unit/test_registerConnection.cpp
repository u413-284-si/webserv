#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockFileSystemPolicy.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class RegisterConnectionTest : public ::testing::Test {
	protected:
	RegisterConnectionTest()
	{
		ConfigServer serverConfig;
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);

		ON_CALL(epollWrapper, addEvent)
		.WillByDefault(Return(true));
	}
	~RegisterConnectionTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockFileSystemPolicy fileSystemPolicy;
	MockSocketPolicy socketPolicy;
	MockProcessOps processOps;
	Server server = Server(configFile, epollWrapper, fileSystemPolicy, socketPolicy, processOps);

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

TEST_F(RegisterConnectionTest, CantOverwriteExistingConnection)
{
	Socket oldClientSocket = {
		"1.1.1.1",
		"11111" };

	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, oldClientSocket), true);
	EXPECT_EQ(server.getConnections().size(), 1);

	// reuse the same dummyFd should fail as a connection with this fd already exists
	EXPECT_EQ(server.registerConnection(serverSock, dummyFd, clientSocket), false);
	EXPECT_EQ(server.getConnections().size(), 1);
}
