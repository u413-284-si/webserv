#include "test_helpers.hpp"

using ::testing::HasSubstr;
using ::testing::Return;

class connectionHandleTimeoutTest : public ServerTestBase {
protected:
	connectionHandleTimeoutTest() { m_connection.m_status = Connection::ReceiveHeader; }
	~connectionHandleTimeoutTest() override { }

	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	const int m_dummyFd = 10;
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
	HTTPRequest& m_request = m_connection.m_request;
};

TEST_F(connectionHandleTimeoutTest, TimeoutPartialSend)
{
	EXPECT_CALL(m_socketOps, writeToSocket).WillOnce(Return(1));

	connectionHandleTimeout(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusRequestTimeout);
	EXPECT_TRUE(m_request.shallCloseConnection);
	EXPECT_EQ(m_connection.m_status, Connection::SendResponse);
	EXPECT_THAT(m_connection.m_buffer, HasSubstr("Content-Type: text/html"));
	EXPECT_THAT(m_connection.m_buffer, HasSubstr("Timeout"));
}
