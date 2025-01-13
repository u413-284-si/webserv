#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockProcessOps.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class HandleCompleteRequestHeaderTest : public ::testing::Test {
protected:
	HandleCompleteRequestHeaderTest()
		: m_server(m_configFile, m_epollWrapper, m_socketPolicy, processOps)
	{
		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(m_epollWrapper, modifyEvent).WillByDefault(Return(true));

		m_server.registerConnection(m_serverSock, m_dummyFd, Socket());
		m_connection = m_server.getConnections().at(m_dummyFd);

		m_connection.m_timeSinceLastEvent = 0;
		m_connection.m_status = Connection::ReceiveHeader;
	}
	~HandleCompleteRequestHeaderTest() override { }

	ConfigFile m_configFile = createDummyConfig();
	NiceMock<MockEpollWrapper> m_epollWrapper;
	MockSocketPolicy m_socketPolicy;
	MockProcessOps processOps;
	Server m_server;

	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
    const int m_dummyFd = 10;

	Connection temp = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
	Connection& m_connection = temp;

};

TEST_F(HandleCompleteRequestHeaderTest, GETRequest)
{
	m_connection.m_buffer.assign("GET / HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, NotAllowedMethod)
{
	m_connection.m_buffer.assign("POST / HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusMethodNotAllowed);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, POSTRequest)
{
	m_connection.m_buffer.assign("POST / HTTP/1.1\r\nHost:example.com\r\nContent-Length:12\r\n\r\nThis is body");
	m_configFile.servers[0].locations[0].allowMethods[MethodPost] = true;

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(m_connection.m_buffer, "This is body");
}

TEST_F(HandleCompleteRequestHeaderTest, ParseFail)
{
	m_connection.m_buffer.assign("Wrong");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, ConfigFileRandomlyDestroyed)
{
	m_connection.m_buffer.assign("GET / HTTP/1.1\r\nHost:example.com\r\n\r\n");
	m_configFile.servers.clear();

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_server.getConnections().size(), 0);
}
