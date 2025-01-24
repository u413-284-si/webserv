#include "test_helpers.hpp"

using ::testing::Return;

class RegisterCGITest : public ServerTestBase {
protected:
	RegisterCGITest() { ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true)); }
	~RegisterCGITest() override { }

	const int dummyFd = 10;

	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
	Connection m_connection = Connection(serverSock, clientSocket, dummyFd, m_configFile.servers);
};

TEST_F(RegisterCGITest, CGIRegisterSuccess)
{
	// Arrange

	// Act & Assert
	EXPECT_TRUE(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, m_connection));

	EXPECT_EQ(m_server.getCGIConnections().size(), 1);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_serverSocket.host, serverSock.host);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_serverSocket.port, serverSock.port);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_clientSocket.host, clientSocket.host);
	EXPECT_EQ(m_server.getCGIConnections().at(dummyFd)->m_clientSocket.port, clientSocket.port);
}

TEST_F(RegisterCGITest, CGIRegisterFail)
{
	// Arrange
	EXPECT_CALL(m_epollWrapper, addEvent).Times(1).WillOnce(Return(false));

	// Act & Assert
	EXPECT_FALSE(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, m_connection));

	EXPECT_EQ(m_server.getCGIConnections().size(), 0);
}

TEST_F(RegisterCGITest, CantOverwriteExistingCGIConnection)
{
	// Arrange
	
	// Act & Assert
	EXPECT_TRUE(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, m_connection));
	EXPECT_FALSE(m_server.registerCGIFileDescriptor(dummyFd, EPOLLIN, m_connection));

	EXPECT_EQ(m_server.getCGIConnections().size(), 1);
}
