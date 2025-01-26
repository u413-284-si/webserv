#include "test_helpers.hpp"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionReceiveHeaderTest : public ServerTestBase {
protected:
	ConnectionReceiveHeaderTest()
	{
		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(m_epollWrapper, modifyEvent).WillByDefault(Return(true));

		m_server.registerConnection(m_serverSock, dummyFd, Socket());
		connection = &m_server.getConnections().at(dummyFd);
		connection->m_timeSinceLastEvent = 0;
		connection->m_status = Connection::ReceiveHeader;
	}
	~ConnectionReceiveHeaderTest() override { }

	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	const int dummyFd = 10;
	Connection* connection;
};

TEST_F(ConnectionReceiveHeaderTest, ReceiveFullRequest)
{
	const char* request = "GET / HTTP/1.0\r\n\r\n";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(m_socketOps, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(m_server, dummyFd, *connection);

	EXPECT_EQ(connection->m_buffer.size(), requestSize);
	EXPECT_EQ(connection->m_request.method, MethodGet);
	EXPECT_NE(connection->m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection->m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveHeaderTest, ReceivePartialRequest)
{
	const char* request = "GET / HTTP/1.";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(m_socketOps, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(m_server, dummyFd, *connection);

	EXPECT_STREQ(connection->m_buffer.c_str(), request);
	EXPECT_EQ(connection->m_buffer.size(), requestSize);
	EXPECT_NE(connection->m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection->m_status, Connection::ReceiveHeader);
}

TEST_F(ConnectionReceiveHeaderTest, RecvFail)
{
	EXPECT_CALL(m_socketOps, readFromSocket).Times(1).WillOnce(Return(-1));

	connectionReceiveHeader(m_server, dummyFd, *connection);

	EXPECT_EQ(m_server.getConnections().size(), 0);
}

TEST_F(ConnectionReceiveHeaderTest, RecvReturnedZero)
{
	EXPECT_CALL(m_socketOps, readFromSocket).Times(1).WillOnce(Return(0));

	connectionReceiveHeader(m_server, dummyFd, *connection);

	EXPECT_EQ(m_server.getConnections().size(), 0);
}

TEST_F(ConnectionReceiveHeaderTest, RequestSizeTooBig)
{
	const char* request = "AAAA";
	const ssize_t requestSize = strlen(request) + 1;

	connection->m_buffer = std::string(995, 'A');

	EXPECT_CALL(m_socketOps, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(m_server, dummyFd, *connection);

	EXPECT_EQ(connection->m_buffer.size(), 1000);
	EXPECT_NE(connection->m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection->m_status, Connection::BuildResponse);
	EXPECT_EQ(connection->m_request.httpStatus, StatusRequestHeaderFieldsTooLarge);
}
