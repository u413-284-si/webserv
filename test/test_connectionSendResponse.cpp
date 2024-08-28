#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionSendResponseTest : public ::testing::Test {
protected:
	ConnectionSendResponseTest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		ON_CALL(epollWrapper, modifyEvent).WillByDefault(Return(true));

		connection.m_timeSinceLastEvent = 0;
		connection.m_buffer = response;
		connection.m_bytesReceived = 123;
		connection.m_status = Connection::SendResponse;
	}
	~ConnectionSendResponseTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	Connection connection;
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nABCD";

	const int dummyFd = 10;
};

TEST_F(ConnectionSendResponseTest, SendFullResponseKeepAlive)
{
	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_bytesReceived, 0);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::ReceiveHeader);
}

TEST_F(ConnectionSendResponseTest, SendFullResponseCloseConnection)
{
	connection.m_request.shallCloseConnection = true;

	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, response);
	EXPECT_EQ(connection.m_bytesReceived, 123);
	EXPECT_EQ(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}

TEST_F(ConnectionSendResponseTest, SendPartialResponse)
{
	std::string partialResponse = "ntent-Length: 4\r\n\r\nABCD";

	EXPECT_CALL(socketPolicy, writeToSocket)
		.Times(1)
		.WillOnce(Return(connection.m_buffer.size() - partialResponse.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, partialResponse);
	EXPECT_EQ(connection.m_bytesReceived, 123);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::SendResponse);
}

TEST_F(ConnectionSendResponseTest, SendFail)
{
	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(-1));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, response);
	EXPECT_EQ(connection.m_bytesReceived, 123);
	EXPECT_EQ(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::Closed);
}
