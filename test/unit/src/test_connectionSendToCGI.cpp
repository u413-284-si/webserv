#include "test_helpers.hpp"

using ::testing::Return;

class ConnectionSendToCGITest : public ServerTestBase {
protected:
	ConnectionSendToCGITest()
	{
		connection.m_pipeToCGIWriteEnd = dummyPipeFd;
		connection.m_request.body = "test body";

		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(m_epollWrapper, removeEvent).WillByDefault(Return());
	}
	~ConnectionSendToCGITest() override { }

	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
	const int dummyFd = 10;
	Connection connection = Connection(serverSock, clientSocket, dummyFd, m_configFile.servers);

	const int dummyPipeFd = 11;
};

TEST_F(ConnectionSendToCGITest, EmptyBody)
{
	// Arrange
	connection.m_request.body = "";

	// Act
	connectionSendToCGI(m_server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, WriteError)
{
	// Arrange
	EXPECT_CALL(m_processOps, writeProcess).Times(1).WillOnce(Return(-1));

	// Act
	connectionSendToCGI(m_server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, FullBodySent)
{
	// Arrange
	const ssize_t bodySize = connection.m_request.body.size();

	EXPECT_CALL(m_processOps, writeProcess).Times(1).WillOnce(Return(bodySize));

	// Act
	connectionSendToCGI(m_server, connection);

	// Assert
	EXPECT_TRUE(connection.m_request.body.empty());
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);
}
