#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "Connection.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "MockProcessOps.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class ConnectionSendToCGITest : public ::testing::Test {
protected:
	ConnectionSendToCGITest()
		: server(configFile, epollWrapper, socketPolicy, processOps)
	{
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);
        connection.m_request.body = "test body";

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));

		ON_CALL(epollWrapper, removeEvent).WillByDefault(Return());
	}
	~ConnectionSendToCGITest() override { }

    const int dummyFd = 10;
	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
    MockProcessOps processOps;
	Server server;
	ConfigServer serverConfig;
   	Connection connection = Connection(serverSock, clientSocket, dummyFd, configFile.servers);
};

TEST_F(ConnectionSendToCGITest, EmptyBody)
{
	// Arrange
	connection.m_request.body = "";

	// Act
	connectionSendToCGI(server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, WriteError)
{
	// Arrange
 	EXPECT_CALL(processOps, writeProcess).Times(1).WillOnce(Return(-1));

	// Act
	connectionSendToCGI(server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, FullBodySent)
{
	// Arrange
    const ssize_t bodySize = connection.m_request.body.size();

	EXPECT_CALL(processOps, writeProcess)
        .Times(1)
		.WillOnce(Return(bodySize));

	// Act
	connectionSendToCGI(server, connection);

	// Assert
	EXPECT_TRUE(connection.m_request.body.empty());
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);
}
