#include "test_helpers.hpp"

using ::testing::Return;

class HandleEventTest : public ServerTestBase {
protected:
	HandleEventTest() { ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true)); }
	~HandleEventTest() override { }

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;
	const int dummyFd4 = 40;

	const Socket serverSock = { .host = m_configFile.servers[0].host, .port = m_configFile.servers[0].port };
	const Socket clientSock = { "192.168.0.1", "1234" };
};

TEST_F(HandleEventTest, ErrorConditionOnVirtualServer)
{
	m_server.registerVirtualServer(dummyFd, serverSock);
	struct epoll_event dummyEvent;
	dummyEvent.events = EPOLLERR;
	dummyEvent.data.fd = dummyFd;

	handleEvent(m_server, dummyEvent);

	EXPECT_EQ(m_server.getVirtualServers().size(), 0);
}

TEST_F(HandleEventTest, EventOnVirtualServer)
{
	EXPECT_CALL(m_socketOps, acceptSingleConnection).WillOnce(Return(-2));
	m_server.registerVirtualServer(dummyFd, serverSock);
	struct epoll_event dummyEvent;
	dummyEvent.events = EPOLLIN;
	dummyEvent.data.fd = dummyFd;

	handleEvent(m_server, dummyEvent);

	EXPECT_EQ(m_server.getVirtualServers().size(), 1);
}

TEST_F(HandleEventTest, EventOnCGIConnection)
{
	m_server.registerConnection(serverSock, dummyFd, clientSock);
	m_server.getConnections().at(dummyFd).m_status = Connection::SendToCGI;
	m_server.registerCGIFileDescriptor(dummyFd2, EPOLLIN, m_server.getConnections().at(dummyFd));
	struct epoll_event dummyEvent;
	dummyEvent.events = EPOLLIN;
	dummyEvent.data.fd = dummyFd2;

	handleEvent(m_server, dummyEvent);

	EXPECT_EQ(m_server.getConnections().size(), 1);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::BuildResponse);
}

TEST_F(HandleEventTest, EventEPOLLERROnConnection)
{
	m_server.registerConnection(serverSock, dummyFd, clientSock);
	m_server.getConnections().at(dummyFd).m_status = Connection::Closed;
	struct epoll_event dummyEvent;
	dummyEvent.events = EPOLLERR;
	dummyEvent.data.fd = dummyFd;

	handleEvent(m_server, dummyEvent);

	EXPECT_EQ(m_server.getConnections().size(), 1);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::Closed);
}

TEST_F(HandleEventTest, EventEPOLLHUPOnConnection)
{
	m_server.registerConnection(serverSock, dummyFd, clientSock);
	m_server.getConnections().at(dummyFd).m_status = Connection::Timeout;
	struct epoll_event dummyEvent;
	dummyEvent.events = EPOLLHUP;
	dummyEvent.data.fd = dummyFd;

	handleEvent(m_server, dummyEvent);

	EXPECT_EQ(m_server.getConnections().size(), 1);
	EXPECT_EQ(m_server.getConnections().at(dummyFd).m_status, Connection::SendResponse);
}
