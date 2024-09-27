#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "Connection.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class ConnectionReceiveFromCGITest : public ::testing::Test {
protected:
	ConnectionReceiveFromCGITest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));

		ON_CALL(epollWrapper, removeEvent).WillByDefault(Return());
	}
	~ConnectionReceiveFromCGITest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	ConfigServer serverConfig;
	const int dummyFd = 10;

	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
};

TEST_F(ConnectionReceiveFromCGITest, ReadError)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);
	connection.m_status = Connection::ReceiveFromCGI;
	int pipefd[2];
	pipe(pipefd);
	connection.m_pipeFromCGIReadEnd = pipefd[0];
	close(pipefd[1]);
	close(pipefd[0]); // Simulate read error

	// Act
	connectionReceiveFromCGI(server, pipefd[0], connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveFromCGITest, PartialRead)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);
	connection.m_status = Connection::ReceiveFromCGI;
	int pipefd[2];
	pipe(pipefd);
	connection.m_pipeFromCGIReadEnd = pipefd[0];
	write(pipefd[1], "CGI response", 12);

	// Act
	connectionReceiveFromCGI(server, pipefd[0], connection);

	// Assert
	EXPECT_EQ(connection.m_request.body, "CGI response");
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);

	close(pipefd[0]);
	close(pipefd[1]);
}
