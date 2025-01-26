#include "test_helpers.hpp"

using ::testing::Return;

class ShutdownServerTest : public ServerTestBase {
protected:
	ShutdownServerTest()
	{
		struct epoll_event dummyEvent;
		dummyEvent.events = EPOLLIN;
		dummyEvent.data.fd = dummyFd;
		dummyEventsVector.push_back(dummyEvent);

		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(m_epollWrapper, waitForEvents).WillByDefault(Return(0));
		ON_CALL(m_epollWrapper, eventsBegin).WillByDefault(Return((dummyEventsVector.begin())));

		g_signalStatus = SIGQUIT;
	}
	~ShutdownServerTest() override { }

	std::vector<struct epoll_event> dummyEventsVector;

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;

	const Socket serverSock = { .host = m_configFile.servers[0].host, .port = m_configFile.servers[0].port };
	const Socket clientSock = { "192.168.0.1", "1234" };
};

TEST_F(ShutdownServerTest, OnlyIdleConnections)
{
	m_server.registerVirtualServer(dummyFd, clientSock);
	m_server.registerConnection(serverSock, dummyFd2, clientSock);
	m_server.registerConnection(serverSock, dummyFd3, clientSock);

	m_server.getConnections().at(dummyFd2).m_status = Connection::Idle;
	m_server.getConnections().at(dummyFd3).m_status = Connection::Idle;

	shutdownServer(m_server);

	EXPECT_EQ(m_server.getConnections().size(), 0);
}

TEST_F(ShutdownServerTest, NonIdleConnection)
{
	m_server.registerConnection(serverSock, dummyFd, clientSock);

	m_server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	EXPECT_CALL(m_epollWrapper, waitForEvents).WillOnce(Return(1));
	EXPECT_CALL(m_socketOps, readFromSocket).WillOnce(Return(0));

	shutdownServer(m_server);

	EXPECT_EQ(m_server.getConnections().size(), 0);
}

TEST_F(ShutdownServerTest, InterruptedBySignal)
{
	m_server.registerConnection(serverSock, dummyFd, clientSock);

	m_server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	g_signalStatus = SIGINT;

	shutdownServer(m_server);

	EXPECT_EQ(m_server.getConnections().size(), 1);
}
