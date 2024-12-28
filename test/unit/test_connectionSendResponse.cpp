#include "test_helpers.hpp"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionSendResponseTest : public ServerTestBase {
protected:
	ConnectionSendResponseTest()
	{
		ON_CALL(m_epollWrapper, modifyEvent)
			.WillByDefault(Return(true));

		connection.m_timeSinceLastEvent = 0;
		connection.m_buffer = response;
		connection.m_status = Connection::SendResponse;
	}
	~ConnectionSendResponseTest() override { }

	const int dummyFd = 10;

	Connection connection = Connection(Socket(), Socket(), dummyFd, m_configFile.servers);
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nABCD";
};

TEST_F(ConnectionSendResponseTest, SendFullResponseKeepAlive)
{
	EXPECT_CALL(m_socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(m_server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer.size(), 0);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::Idle);
}

TEST_F(ConnectionSendResponseTest, SendFullResponseCloseConnection)
{
	connection.m_request.shallCloseConnection = true;

	EXPECT_CALL(m_socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(m_server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, response);
	EXPECT_EQ(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionSendResponseTest, SendPartialResponse)
{
	std::string partialResponse = "ntent-Length: 4\r\n\r\nABCD";

	EXPECT_CALL(m_socketPolicy, writeToSocket)
		.Times(1)
		.WillOnce(Return(connection.m_buffer.size() - partialResponse.size()));

	connectionSendResponse(m_server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, partialResponse);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::SendResponse);
}

TEST_F(ConnectionSendResponseTest, SendFail)
{
	EXPECT_CALL(m_socketPolicy, writeToSocket).Times(1).WillOnce(Return(-1));

	connectionSendResponse(m_server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, response);
	EXPECT_EQ(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}
