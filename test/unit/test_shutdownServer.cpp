#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockFileSystemPolicy.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;
ConfigFile createTestConfigfile();

class ShutdownServerTest : public ::testing::Test {
protected:
	ShutdownServerTest()
	{
		struct epoll_event dummyEvent;
		dummyEvent.events = EPOLLIN;
		dummyEvent.data.fd = dummyFd;
		dummyEventsVector.push_back(dummyEvent);

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(epollWrapper, waitForEvents).WillByDefault(Return(0));
		ON_CALL(epollWrapper, eventsBegin).WillByDefault(Return((dummyEventsVector.begin())));

		g_signalStatus = SIGQUIT;
	}
	~ShutdownServerTest() override { }

	ConfigFile configFile = createTestConfigfile();
	NiceMock<MockEpollWrapper> epollWrapper;
	MockFileSystemPolicy fileSystemPolicy;
	MockSocketPolicy socketPolicy;
	MockProcessOps processOps;
	Server server = Server(configFile, epollWrapper, fileSystemPolicy, socketPolicy, processOps);
	std::vector<struct epoll_event> dummyEventsVector;

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;
	const int dummyFd4 = 40;

	const Socket serverSock = {
		.host = configFile.servers[0].host,
		.port = configFile.servers[0].port
	};
	const Socket clientSock = { "192.168.0.1", "1234" };
};

TEST_F(ShutdownServerTest, OnlyClosedAndIdleConnections)
{
	server.registerVirtualServer(dummyFd, clientSock);
	server.registerConnection(serverSock, dummyFd2, clientSock);
	server.registerConnection(serverSock, dummyFd3, clientSock);
	server.registerConnection(serverSock, dummyFd4, clientSock);

	server.getConnections().at(dummyFd2).m_status = Connection::Idle;
	server.getConnections().at(dummyFd3).m_status = Connection::Closed;
	server.getConnections().at(dummyFd4).m_status = Connection::Idle;

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(ShutdownServerTest, NonIdleConnection)
{
	server.registerConnection(serverSock, dummyFd, clientSock);

	server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	EXPECT_CALL(epollWrapper, waitForEvents).WillOnce(Return(1));
	EXPECT_CALL(socketPolicy, readFromSocket).WillOnce(Return(0));

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(ShutdownServerTest, InterruptedBySignal)
{
	server.registerConnection(serverSock, dummyFd, clientSock);

	server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	g_signalStatus = SIGINT;

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 1);
}
