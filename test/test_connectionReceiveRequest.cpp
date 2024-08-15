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
	}
	~ConnectionReceiveRequestTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;

	const int dummyFd = 10;
};

TEST_F(ConnectionReceiveRequestTest, ReceiveFullRequest)
{
	auto request = "GET / HTTP/1.0\r\n\r\n";
	const ssize_t requestSize = strlen(request) + 1;

	EXPECT_CALL(socketPolicy, readFromSocket)
	.Times(1)
	.WillOnce(DoAll(SetArrayArgument<1>(request, request + requestSize),
					Return(requestSize)));

	Connection connection;

	connectionReceiveRequest(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, "");
	EXPECT_EQ(connection.m_request.method, MethodGet);
	EXPECT_NE(connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}
