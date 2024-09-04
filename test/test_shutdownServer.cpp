#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/epoll.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"
#include "signalHandler.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class ServerShutdownTest : public ::testing::Test {
protected:
	ServerShutdownTest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		struct epoll_event dummyEvent;
		dummyEvent.events = EPOLLIN;
		dummyEvent.data.fd = dummyFd;
		dummyEventsVector.push_back(dummyEvent);

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(epollWrapper, waitForEvents).WillByDefault(Return(0));
		ON_CALL(epollWrapper, eventsBegin).WillByDefault(Return((dummyEventsVector.begin())));
	}
	~ServerShutdownTest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	std::vector<struct epoll_event> dummyEventsVector;

	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;
	const int dummyFd4 = 40;

	const Socket dummySocket = { "1.1.1.1", "8080" };
};

TEST_F(ServerShutdownTest, OnlyClosedAndIdleConnections)
{
	server.registerVirtualServer(dummyFd, dummySocket);
	server.registerConnection(dummySocket, dummyFd2, dummySocket);
	server.registerConnection(dummySocket, dummyFd3, dummySocket);
	server.registerConnection(dummySocket, dummyFd4, dummySocket);

	server.getConnections().at(dummyFd2).m_status = Connection::Idle;
	server.getConnections().at(dummyFd3).m_status = Connection::Closed;
	server.getConnections().at(dummyFd4).m_status = Connection::Idle;

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(ServerShutdownTest, NonIdleConnection)
{
	server.registerConnection(dummySocket, dummyFd, dummySocket);

	server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	EXPECT_CALL(epollWrapper, waitForEvents).WillOnce(Return(1));
	EXPECT_CALL(socketPolicy, readFromSocket).WillOnce(Return(0));

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(ServerShutdownTest, InterruptedBySignal)
{
	server.registerConnection(dummySocket, dummyFd, dummySocket);

	server.getConnections().at(dummyFd).m_status = Connection::ReceiveHeader;

	g_signalStatus = SIGINT;

	shutdownServer(server);

	EXPECT_EQ(server.getConnections().size(), 1);
}
