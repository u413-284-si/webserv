#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "Server.hpp"

using ::testing::Return;
using ::testing::NiceMock;

class HandleTimeoutTest : public ::testing::Test {
	protected:
	HandleTimeoutTest()
	{
		ON_CALL(epollWrapper, modifyEvent)
		.WillByDefault(Return(true));
	}
	~HandleTimeoutTest() override { }

	std::map<int, Connection> connections;
	NiceMock<MockEpollWrapper> epollWrapper;
	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;
};


TEST_F(HandleTimeoutTest, Timeout)
{
	time_t clientTimeout = -1;

	Connection connection;
	connection.m_timeSinceLastEvent = std::time(0);
	connections[dummyFd] = connection;

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 0);
}

TEST_F(HandleTimeoutTest, NoTimeout)
{
	time_t clientTimeout = 100;

	Connection connection;
	connection.m_timeSinceLastEvent = std::time(0);
	connections[dummyFd] = connection;

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 1);
}

TEST_F(HandleTimeoutTest, MultipleTimeouts)
{
	time_t clientTimeout = -1;

	Connection connection;
	connection.m_timeSinceLastEvent = std::time(0);
	connections[dummyFd] = connection;
	connections[dummyFd2] = connection;
	connections[dummyFd3] = connection;

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 0);
}
