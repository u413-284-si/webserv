#include "test_helpers.hpp"

using ::testing::Return;
using ::testing::HasSubstr;

class connectionBuildResponseTest : public ServerTestBase {
protected:
	connectionBuildResponseTest() { m_connection.m_status = Connection::BuildResponse; }
	~connectionBuildResponseTest() override { }

	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	const int m_dummyFd = 10;
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
	HTTPRequest& m_request = m_connection.m_request;
};

TEST_F(connectionBuildResponseTest, SuccessfulBuildPartialSend)
{
	EXPECT_CALL(m_fileSystemOps, getFileContents).WillOnce(Return("Hello World"));
	EXPECT_CALL(m_socketOps, writeToSocket).WillOnce(Return(1));

	m_request.method = MethodGet;
	m_request.targetResource = "/something.html";

	connectionBuildResponse(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_connection.m_status, Connection::SendResponse);
	EXPECT_THAT(m_connection.m_buffer, HasSubstr("Content-Type: text/html"));
	EXPECT_THAT(m_connection.m_buffer, HasSubstr("Hello World"));
}
