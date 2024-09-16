#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionReceiveHeaderTest : public ::testing::Test {
protected:
	ConnectionReceiveHeaderTest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, modifyEvent)
			.WillByDefault(Return(true));

		connection.m_timeSinceLastEvent = 0;
		connection.m_status = Connection::ReceiveHeader;
	}
	~ConnectionReceiveHeaderTest() override { }

	ConfigFile configFile = createDummyConfig();
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	Socket m_serverSock = {
		.host = "127.0.0.1",
		.port = "8080"
	};

	Connection connection = Connection(m_serverSock, Socket(), configFile.servers);

	const int dummyFd = 10;
};

TEST_F(ConnectionReceiveHeaderTest, ReceiveFullRequest)
{
	const char* request = "GET / HTTP/1.0\r\n\r\n";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(socketPolicy, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(server, dummyFd, connection);

	EXPECT_EQ(connection.m_bytesReceived, requestSize);
	EXPECT_EQ(connection.m_request.method, MethodGet);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveHeaderTest, ReceivePartialRequest)
{
	const char* request = "GET / HTTP/1.";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(socketPolicy, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, request);
	EXPECT_EQ(connection.m_bytesReceived, requestSize);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::ReceiveHeader);
}

TEST_F(ConnectionReceiveHeaderTest, RecvFail)
{
	EXPECT_CALL(socketPolicy, readFromSocket).Times(1).WillOnce(Return(-1));

	connectionReceiveHeader(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionReceiveHeaderTest, RecvReturnedZero)
{
	EXPECT_CALL(socketPolicy, readFromSocket).Times(1).WillOnce(Return(0));

	connectionReceiveHeader(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionReceiveHeaderTest, RequestSizeTooBig)
{
	const char* request = "AAAA";
	const ssize_t requestSize = strlen(request) + 1;

	connection.m_buffer = std::string(995, 'A');
	connection.m_bytesReceived = 995;

	EXPECT_CALL(socketPolicy, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize), Return(requestSize)));

	connectionReceiveHeader(server, dummyFd, connection);

	EXPECT_EQ(connection.m_bytesReceived, 1000);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(connection.m_request.httpStatus, StatusRequestHeaderFieldsTooLarge);
}
