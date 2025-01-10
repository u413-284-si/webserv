#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionSendResponseTest : public ::testing::Test {
protected:
	ConnectionSendResponseTest()
		: server(configFile, epollWrapper, socketPolicy, processOps)
	{
		ON_CALL(epollWrapper, addEvent)
			.WillByDefault(Return(true));
		ON_CALL(epollWrapper, modifyEvent).WillByDefault(Return(true));

		server.registerConnection(serverSock, dummyFd, Socket());
		connection = server.getConnections().at(dummyFd);

		connection.m_timeSinceLastEvent = 0;
		connection.m_buffer = response;
		connection.m_status = Connection::SendResponse;
	}
	~ConnectionSendResponseTest() override { }

	ConfigFile configFile = createDummyConfig();
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
    MockProcessOps processOps;
	Server server;

	Socket serverSock = {
		.host = "127.0.0.1",
		.port = "8080"
	};
	const int dummyFd = 10;

	Connection temp = Connection(serverSock, Socket(), dummyFd, configFile.servers);
	Connection& connection = temp;

	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nABCD";
};

TEST_F(ConnectionSendResponseTest, SendFullResponseKeepAlive)
{
	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer.size(), 0);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::Idle);
}

TEST_F(ConnectionSendResponseTest, SendFullResponseCloseConnection)
{
	connection.m_request.shallCloseConnection = true;

	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(connection.m_buffer.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(server.getConnections().size(), 0);
}

TEST_F(ConnectionSendResponseTest, SendPartialResponse)
{
	std::string partialResponse = "ntent-Length: 4\r\n\r\nABCD";

	EXPECT_CALL(socketPolicy, writeToSocket)
		.Times(1)
		.WillOnce(Return(connection.m_buffer.size() - partialResponse.size()));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(connection.m_buffer, partialResponse);
	EXPECT_EQ(connection.m_timeSinceLastEvent, std::time(0));
	EXPECT_EQ(connection.m_status, Connection::SendResponse);
}

TEST_F(ConnectionSendResponseTest, SendFail)
{
	EXPECT_CALL(socketPolicy, writeToSocket).Times(1).WillOnce(Return(-1));

	connectionSendResponse(server, dummyFd, connection);

	EXPECT_EQ(server.getConnections().size(), 0);
}
