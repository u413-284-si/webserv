#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Server.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionReceiveBodyTest : public ::testing::Test {
protected:
	ConnectionReceiveBodyTest()
	{
		ON_CALL(m_epollWrapper, modifyEvent)
			.WillByDefault(Return(true));

		m_connection.m_status = Connection::ReceiveBody;
	}
	~ConnectionReceiveBodyTest() override { }

	ConfigFile m_configFile = createDummyConfig();
	NiceMock<MockEpollWrapper> m_epollWrapper;
	MockSocketPolicy m_socketPolicy;
	MockProcessOps m_processOps;
	Server m_server = Server(m_configFile, m_epollWrapper, m_socketPolicy, m_processOps);

	Socket m_serverSock = {
		.host = "127.0.0.1",
		.port = "8080"
	};
	const int m_dummyFd = 10;
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
};

TEST_F(ConnectionReceiveBodyTest, ReceiveFullBody)
{
	// Arrange
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.contentLength = bodySize;

	// Act
	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	// Assert
	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveBodyTest, ReceiveFullBodyForCGI)
{
	// Arrange
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.contentLength = bodySize;
	m_connection.m_request.hasCGI = true;

	// Act
	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	// Assert
	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::SendToCGI);
}

TEST_F(ConnectionReceiveBodyTest, ReceivePartialBody)
{
	// Arrange
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.contentLength = bodySize + 1; // Simulate more data to come 

	// Act
	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	// Assert
	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::ReceiveBody);
}

TEST_F(ConnectionReceiveBodyTest, ContentLengthIsTooSmall)
{
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.headers["content-length"] = std::to_string(1);

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(m_connection.m_request.httpStatus, StatusBadRequest);
}

TEST_F(ConnectionReceiveBodyTest, MaxBodySizeReached)
{
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.contentLength = bodySize + 1;
	m_configFile.servers[0].locations[0].maxBodySize = 1;

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(m_connection.m_request.httpStatus, StatusRequestEntityTooLarge);
}

TEST_F(ConnectionReceiveBodyTest, RecvFail)
{
	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(Return(-1));

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), 0);
	EXPECT_EQ(m_connection.m_status, Connection::Closed);
}

TEST_F(ConnectionReceiveBodyTest, RecvReturnedZero)
{
	EXPECT_CALL(m_socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(Return(0));

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), 0);
	EXPECT_EQ(m_connection.m_status, Connection::Closed);
}

