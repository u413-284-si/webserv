#include "test_helpers.hpp"
#include <unistd.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArrayArgument;

class ConnectionReceiveFromCGITest : public ServerTestBase {
protected:
	ConnectionReceiveFromCGITest()
	{
		connection.m_status = Connection::ReceiveFromCGI;
		connection.m_pipeFromCGIReadEnd = pipefd[0];

		ON_CALL(m_epollWrapper, addEvent)
			.WillByDefault(Return(true));

		ON_CALL(m_epollWrapper, removeEvent)
			.WillByDefault(Return());
	}
	~ConnectionReceiveFromCGITest() override { }

	const int dummyFd = 10;
	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };

	Connection connection = Connection(serverSock, clientSocket, dummyFd, m_configFile.servers);
	int pipefd[2];

    void SetUp() override {
        // Create a pipe
        pipe(pipefd);
    }

    void TearDown() override {
        close(pipefd[0]);
        close(pipefd[1]);
    }
};

TEST_F(ConnectionReceiveFromCGITest, ReadError)
{
	// Arrange
	EXPECT_CALL(m_processOps, readProcess).Times(1).WillOnce(Return(-1));

	// Act
	connectionReceiveFromCGI(m_server, pipefd[0], connection);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
}

TEST_F(ConnectionReceiveFromCGITest, PartialRead)
{
	// Arrange
	const char* response = "CGI response";
    const ssize_t responseSize = strlen(response);

	EXPECT_CALL(m_processOps, readProcess)
        .Times(1)
		.WillOnce(DoAll(
			SetArrayArgument<1>(response, response + responseSize),
			Return(responseSize)));

	// Act
	connectionReceiveFromCGI(m_server, pipefd[0], connection);

	// Assert
	EXPECT_EQ(connection.m_request.body, response);
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);
}
