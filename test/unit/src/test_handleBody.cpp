#include "test_helpers.hpp"

using ::testing::Return;

class HandleBodyTest : public ServerTestBase {
protected:
	HandleBodyTest()
	{
		m_connection.m_status = Connection::ReceiveBody;
	}
	~HandleBodyTest() override { }

	const int m_dummyFd = 10;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
};

TEST_F(HandleBodyTest, ParseBodyFailed)
{
	EXPECT_CALL(m_epollWrapper, modifyEvent).WillOnce(Return(false));

	m_connection.m_request.isChunked = true;
	m_connection.m_buffer.assign("wrong encoding\r\n0\r\n\r\n");

	handleBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusBadRequest);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

