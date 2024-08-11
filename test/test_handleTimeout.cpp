#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "Server.hpp"

using ::testing::Return;

class HandleTimeoutTest : public ::testing::Test {
	protected:
	HandleTimeoutTest() { }
	~HandleTimeoutTest() override { }

	std::map<int, Connection> connections;
	MockEpollWrapper epollWrapper;
	const int dummyFd = 10;
	const int dummyFd2 = 20;
	const int dummyFd3 = 30;
};


TEST_F(HandleTimeoutTest, Timeout)
{
	time_t clientTimeout = -1;

	Connection connection;
	connection.updateTimeSinceLastEvent();
	connections[dummyFd] = connection;

	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(1);

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 0);
}

TEST_F(HandleTimeoutTest, NoTimeout)
{
	time_t clientTimeout = 100;

	Connection connection;
	connection.updateTimeSinceLastEvent();
	connections[dummyFd] = connection;

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 1);
}

TEST_F(HandleTimeoutTest, MultipleTimeouts)
{
	time_t clientTimeout = -1;

	Connection connection;
	connection.updateTimeSinceLastEvent();
	connections[dummyFd] = connection;
	connections[dummyFd2] = connection;
	connections[dummyFd3] = connection;

	EXPECT_CALL(epollWrapper, removeEvent)
	.Times(3);

	handleTimeout(connections, clientTimeout, epollWrapper);

	EXPECT_EQ(connections.size(), 0);
}
