#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "Connection.hpp"
#include "MockEpollWrapper.hpp"
#include "MockProcessOps.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionReceiveFromCGITest : public ::testing::Test {
protected:
	ConnectionReceiveFromCGITest()
		: server(configFile, epollWrapper, socketPolicy, processOps)
	{
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);
		connection.m_status = Connection::ReceiveFromCGI;

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));

		ON_CALL(epollWrapper, removeEvent).WillByDefault(Return());
	}
	~ConnectionReceiveFromCGITest() override { }

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

TEST_F(ConnectionReceiveFromCGITest, ReadError)
{
	// Arrange
	EXPECT_CALL(processOps, readProcess).Times(1).WillOnce(Return(-1));

	// Act
	connectionReceiveFromCGI(server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveFromCGITest, PartialRead)
{
	// Arrange
	const char* response = "CGI response";
    const ssize_t responseSize = strlen(response);

	EXPECT_CALL(processOps, readProcess)
        .Times(1)
		.WillOnce(DoAll(
			SetArrayArgument<1>(response, response + responseSize),
			Return(responseSize)));

	// Act
	connectionReceiveFromCGI(server, connection);

	// Assert
	EXPECT_EQ(connection.m_request.body, response);
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);
}
