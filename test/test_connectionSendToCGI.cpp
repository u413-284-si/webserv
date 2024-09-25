#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "Connection.hpp"
#include "MockEpollWrapper.hpp"
#include "MockSocketPolicy.hpp"
#include "Server.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class ConnectionSendToCGITest : public ::testing::Test {
protected:
	ConnectionSendToCGITest()
		: server(configFile, epollWrapper, socketPolicy)
	{
		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);

		ON_CALL(epollWrapper, addEvent).WillByDefault(Return(true));

		ON_CALL(epollWrapper, removeEvent).WillByDefault(Return());
	}
	~ConnectionSendToCGITest() override { }

	ConfigFile configFile;
	NiceMock<MockEpollWrapper> epollWrapper;
	MockSocketPolicy socketPolicy;
	Server server;
	ConfigServer serverConfig;
	const int dummyFd = 10;
	const int dummyPipeFd = 11;

	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
};

TEST_F(ConnectionSendToCGITest, ConnectionSendToCGI_EmptyBody)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);
	connection.m_request.body = "";

	// Act
	connectionSendToCGI(server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, ConnectionSendToCGI_WriteError)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);
	connection.m_request.body = "test body";
	std::cout << "Clientfd: " << connection.m_clientFd << std::endl;

	// Act
	connectionSendToCGI(server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionSendToCGITest, ConnectionSendToCGI_FullBodySent)
{
	// Arrange
	Connection connection(serverSock, clientSocket, dummyFd, configFile.servers);
	int pipefd[2];
	pipe(pipefd);
	connection.m_pipeToCGIWriteEnd = pipefd[1];
	connection.m_request.body = "test body";

	// Act
	connectionSendToCGI(server, pipefd[1], connection);

	// Assert
	EXPECT_TRUE(connection.m_request.body.empty());
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);

	close(pipefd[0]);
	close(pipefd[1]);
}
