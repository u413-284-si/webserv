#include "test_helpers.hpp"

using ::testing::Return;

class RegisterConnectionTest : public ServerTestBase {
	protected:
	RegisterConnectionTest()
	{
		ON_CALL(m_epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}
	~RegisterConnectionTest() override { }

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
	EXPECT_EQ(m_server.registerConnection(serverSock, dummyFd, clientSocket), true);
	EXPECT_EQ(m_server.getConnections().size(), 1);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_serverSocket.host, serverSock.host);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_serverSocket.port, serverSock.port);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_clientSocket.host, clientSocket.host);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_clientSocket.port, clientSocket.port);
}

TEST_F(RegisterConnectionTest, ConnectionRegisterFail)
{
	EXPECT_CALL(m_epollWrapper, addEvent)
	.Times(1)
	.WillOnce(Return(false));

	EXPECT_EQ(m_server.registerConnection(serverSock, dummyFd, clientSocket), false);
	EXPECT_EQ(m_server.getConnections().size(), 0);
}

TEST_F(RegisterConnectionTest, CantOverwriteExistingConnection)
{
	Socket oldClientSocket = {
		"1.1.1.1",
		"11111" };

	EXPECT_EQ(m_server.registerConnection(serverSock, dummyFd, oldClientSocket), true);
	EXPECT_EQ(m_server.getConnections().size(), 1);

	// reuse the same dummyFd should fail as a connection with this fd already exists
	EXPECT_EQ(m_server.registerConnection(serverSock, dummyFd, clientSocket), false);
	EXPECT_EQ(m_server.getConnections().size(), 1);
}
