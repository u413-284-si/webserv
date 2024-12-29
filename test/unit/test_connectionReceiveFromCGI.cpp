#include "test_helpers.hpp"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;

class ConnectionReceiveFromCGITest : public ServerTestBase {
protected:
	ConnectionReceiveFromCGITest()
	{
		ON_CALL(m_epollWrapper, removeEvent).WillByDefault(Return());
		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));

		connection.m_status = Connection::ReceiveFromCGI;
		connection.m_request.hasCGI = true;
		connection.m_pipeFromCGIReadEnd = dummyPipeFd;
		connection.m_pipeToCGIWriteEnd = dummyPipeFd;
		connection.m_cgiPid = 1234;
	}
	~ConnectionReceiveFromCGITest() override { }

	Socket serverSock = { "127.0.0.1", "8080" };
	Socket clientSocket = { "192.168.0.1", "12345" };
	const int dummyFd = 10;
	Connection connection = Connection(serverSock, clientSocket, dummyFd, m_configFile.servers);

	const int dummyPipeFd = 11;
};

TEST_F(ConnectionReceiveFromCGITest, ReadError)
{
	// Arrange
	EXPECT_CALL(m_processOps, readProcess).Times(1).WillOnce(Return(-1));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

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
		.WillOnce(DoAll(SetArrayArgument<1>(response, response + responseSize), Return(responseSize)));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_request.body, response);
	EXPECT_EQ(connection.m_status, Connection::ReceiveFromCGI);
}

TEST_F(ConnectionReceiveFromCGITest, FullReadWaitpidError)
{
	// Arrange
	EXPECT_CALL(m_processOps, readProcess).WillOnce(Return(0));
	EXPECT_CALL(m_processOps, waitForProcess).WillOnce(DoAll(SetArgPointee<1>(123), Return(-1)));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(connection.m_pipeFromCGIReadEnd, -1);
	EXPECT_EQ(connection.m_pipeToCGIWriteEnd, -1);
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_FALSE(connection.m_request.hasCGI);
}

TEST_F(ConnectionReceiveFromCGITest, FullReadChildExitSuccess)
{
	// Arrange
	// Status Code is 0 = SUCCESS
	const int exitCode = 0;
	// Set the lower 8 bits of wstatus (exit code) to the desired exit value.
	// Set bits 8–15 of wstatus to zero to indicate normal termination.
	const int wstatus = (exitCode & 0xFF) << 8;
	EXPECT_CALL(m_processOps, readProcess).WillOnce(Return(0));
	EXPECT_CALL(m_processOps, waitForProcess).WillOnce(DoAll(SetArgPointee<1>(wstatus), Return(0)));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(connection.m_pipeFromCGIReadEnd, -1);
	EXPECT_EQ(connection.m_pipeToCGIWriteEnd, -1);
}

TEST_F(ConnectionReceiveFromCGITest, FullReadChildExitFailure)
{
	// Arrange
	// Status Code is non 0 = FAILURE
	const int exitCode = 42;
	// Set the lower 8 bits of wstatus (exit code) to the desired exit value.
	// Set bits 8–15 of wstatus to zero to indicate normal termination.
	const int wstatus = (exitCode & 0xFF) << 8;
	EXPECT_CALL(m_processOps, readProcess).WillOnce(Return(0));
	EXPECT_CALL(m_processOps, waitForProcess).WillOnce(DoAll(SetArgPointee<1>(wstatus), Return(0)));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(connection.m_pipeFromCGIReadEnd, -1);
	EXPECT_EQ(connection.m_pipeToCGIWriteEnd, -1);
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_FALSE(connection.m_request.hasCGI);
}

TEST_F(ConnectionReceiveFromCGITest, FullReadChildReceivedSignal)
{
	// Arrange
	// Status Code is non 0 = FAILURE
	const int signalNumber = SIGKILL;
	;
	// The lower 7 bits of wstatus (bits 0–6) hold the signal number.
	// Bit 7 should be 0 (indicating no core dump).
	const int wstatus = signalNumber & 0x7F;
	EXPECT_CALL(m_processOps, readProcess).WillOnce(Return(0));
	EXPECT_CALL(m_processOps, waitForProcess).WillOnce(DoAll(SetArgPointee<1>(wstatus), Return(0)));

	// Act
	connectionReceiveFromCGI(m_server, dummyPipeFd, connection);

	// Assert
	EXPECT_EQ(connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(connection.m_pipeFromCGIReadEnd, -1);
	EXPECT_EQ(connection.m_pipeToCGIWriteEnd, -1);
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_FALSE(connection.m_request.hasCGI);
}
