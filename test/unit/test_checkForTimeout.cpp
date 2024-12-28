#include "test_helpers.hpp"

using ::testing::Return;

class CheckForTimeoutTest : public ServerTestBase {
protected:
	CheckForTimeoutTest()
	{
		ON_CALL(m_epollWrapper, addEvent)
			.WillByDefault(Return(true));
	}
	~CheckForTimeoutTest() override { }

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;

	const Socket serverSock = { "127.0.0.1", "8080" };
	const Socket dummySocket = { "1.1.1.1", "8080" };
};

TEST_F(CheckForTimeoutTest, Timeout)
{
	m_server.setClientTimeout(-1);

	m_server.registerConnection(serverSock, dummyFd, dummySocket);

	checkForTimeout(m_server);

	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::Timeout);
}

TEST_F(CheckForTimeoutTest, NoTimeout)
{
	m_server.setClientTimeout(100);

	m_server.registerConnection(serverSock, dummyFd, dummySocket);

	checkForTimeout(m_server);

	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::Idle);
}

TEST_F(CheckForTimeoutTest, MultipleTimeouts)
{
	m_server.setClientTimeout(-1);

	m_server.registerConnection(serverSock, dummyFd, dummySocket);
	m_server.registerConnection(serverSock, dummyFd2, dummySocket);
	m_server.registerConnection(serverSock, dummyFd3, dummySocket);

	checkForTimeout(m_server);

	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::Timeout);
	EXPECT_EQ(m_server.getConnections().at(dummyFd2).m_status, Connection::Timeout);
	EXPECT_EQ(m_server.getConnections().at(dummyFd3).m_status, Connection::Timeout);
}
