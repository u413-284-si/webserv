#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::NiceMock;

class ConnectionReceiveRequestTest : public ::testing::Test {
protected:
	ConnectionReceiveRequestTest() : server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, modifyEvent)
		.WillByDefault(Return(true));

		connection.m_timeSinceLastEvent = 0;
	}
	~ConnectionReceiveRequestTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	Connection connection;

	const int dummyFd = 10;
};

TEST_F(ConnectionReceiveRequestTest, ReceiveFullRequest)
{
	const char* request = "GET / HTTP/1.0\r\n\r\n";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize),
					Return(requestSize)));

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, requestSize);
	EXPECT_EQ(connection.m_request.method, MethodGet);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveRequestTest, ReceivePartialRequest)
{
	const char* request = "GET / HTTP/1.";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize),
					Return(requestSize)));

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, request);
	EXPECT_EQ(connection.m_bytesReceived, requestSize);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::ReceiveRequest);
}

TEST_F(ConnectionReceiveRequestTest, RecvFail)
{
	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(Return(-1));

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, 0);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionReceiveRequestTest, RecvReturnedZero)
{
	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(Return(0));

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, 0);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionReceiveRequestTest, RequestSizeTooBig)
{
	const char* request = "AAAA";
	const ssize_t requestSize = strlen(request) + 1;

	connection.m_buffer = std::string(995, 'A');
	connection.m_bytesReceived = 995;

	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize),
					Return(requestSize)));

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_bytesReceived, 1000);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}