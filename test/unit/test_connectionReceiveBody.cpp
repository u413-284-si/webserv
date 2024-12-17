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
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.headers["content-length"] = std::to_string(bodySize);

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveBodyTest, ReceivePartialBody)
{
	const char* body = "This is a body";
	const ssize_t bodySize = strlen(body) + 1;

	EXPECT_CALL(m_socketPolicy, readFromSocket)
		.Times(1)
		.WillOnce(DoAll(SetArrayArgument<1>(body, body + bodySize), Return(bodySize)));

	m_connection.m_request.headers["content-length"] = std::to_string(bodySize + 1);

	connectionReceiveBody(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_buffer.size(), bodySize);
	EXPECT_NE(m_connection.m_timeSinceLastEvent, 0);
	EXPECT_EQ(m_connection.m_status, Connection::ReceiveBody);
}
