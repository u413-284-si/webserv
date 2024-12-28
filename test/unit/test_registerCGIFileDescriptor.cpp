#include "test_helpers.hpp"

using ::testing::Return;

class RegisterCGITest : public ServerTestBase {
	protected:
	RegisterCGITest()
	{
		ON_CALL(m_epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}
	~RegisterCGITest() override { }

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
	Connection connection(serverSock, clientSocket, dummyFd, m_configFile.servers);

	// Act & Assert
	EXPECT_EQ(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, connection), true);
	EXPECT_EQ(m_server.getCGIConnections().size(), 1);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_serverSocket.host, serverSock.host);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_serverSocket.port, serverSock.port);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_clientSocket.host, clientSocket.host);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_clientSocket.port, clientSocket.port);
}

TEST_F(RegisterCGITest, CGIRegisterFail)
{
	// Arrange
	EXPECT_CALL(m_epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	Connection connection(serverSock, clientSocket, dummyFd, m_configFile.servers);

	// Act & Assert
	EXPECT_EQ(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, connection), false);
	EXPECT_EQ(m_server.getCGIConnections().size(), 0);
}
