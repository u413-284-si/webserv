#include "test_helpers.hpp"

using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::SetArrayArgument;

class HandleCompleteRequestHeaderTest : public ServerTestBase {
protected:
	HandleCompleteRequestHeaderTest()
	{
		ON_CALL(m_epollWrapper, addEvent).WillByDefault(Return(true));
		ON_CALL(m_epollWrapper, modifyEvent).WillByDefault(Return(true));

		Location location;
		location.path = "/redirect";
		location.returns = std::make_pair(StatusMovedPermanently, "/secret");
		m_configFile.servers[0].locations.emplace_back(location);

		Location location2;
		location2.path = "/cgi-bin";
		location2.cgiPath = "/usr/bin/bash";
		location2.cgiExt = ".sh";
		location2.allowedMethods[MethodPost] = true;
		m_configFile.servers[0].locations.emplace_back(location2);

		m_connection.m_status = Connection::ReceiveHeader;
	}
	~HandleCompleteRequestHeaderTest() override { }

	const int m_dummyFd = 10;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };

	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
};

TEST_F(HandleCompleteRequestHeaderTest, GETRequest)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));

	m_connection.m_buffer.assign("GET / HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, NotAllowedMethod)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));

	m_connection.m_buffer.assign("POST / HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusMethodNotAllowed);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, POSTRequest)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileNotExist));

	m_connection.m_buffer.assign("POST /new.txt "
								 "HTTP/1.1\r\nHost:example.com\r\nContent-Length:12\r\n\r\nThis is body");
	m_configFile.servers[0].locations[0].allowedMethods[MethodPost] = true;

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
	EXPECT_EQ(m_connection.m_buffer, "This is body");
}

TEST_F(HandleCompleteRequestHeaderTest, ParseFail)
{
	m_connection.m_buffer.assign("Wrong");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, ConfigFileRandomlyDestroyed)
{
	m_connection.m_buffer.assign("GET / HTTP/1.1\r\nHost:example.com\r\n\r\n");
	m_configFile.servers.clear();

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_connection.m_status, Connection::Closed);
}

TEST_F(HandleCompleteRequestHeaderTest, GETRequestHitsLocationWithReturn)
{
	EXPECT_CALL(m_epollWrapper, modifyEvent);

	m_connection.m_buffer.assign("GET /redirect HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusMovedPermanently);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, GETRequestWithCGIhasError)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_processOps, pipeProcess).WillOnce(Return(-1));
	EXPECT_CALL(m_epollWrapper, modifyEvent);

	m_connection.m_buffer.assign("GET /cgi-bin/helloWorld.sh HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}

TEST_F(HandleCompleteRequestHeaderTest, GETRequestWithCGI)
{
	int dummyPipeFds[2] = { 11, 21 };
	pid_t dummyPid = 1234;

	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_processOps, pipeProcess)
		.WillOnce(DoAll(SetArrayArgument<0>(dummyPipeFds, dummyPipeFds + 2), Return(0)));
	EXPECT_CALL(m_processOps, forkProcess).WillOnce(DoAll(SetArgReferee<0>(dummyPid), Return(0)));
	EXPECT_CALL(m_epollWrapper, addEvent);

	m_connection.m_buffer.assign("GET /cgi-bin/helloWorld.sh HTTP/1.1\r\nHost:example.com\r\n\r\n");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_connection.m_status, Connection::ReceiveFromCGI);
}

TEST_F(HandleCompleteRequestHeaderTest, POSTRequestWithCGIhasError)
{
	int dummyPipeFds[2] = { 11, 21 };
	int dummyPipeFds2[2] = { 12, 22 };
	pid_t dummyPid = 1234;

	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_processOps, pipeProcess)
		.WillOnce(DoAll(SetArrayArgument<0>(dummyPipeFds2, dummyPipeFds2 + 2), Return(0)))
		.WillOnce(DoAll(SetArrayArgument<0>(dummyPipeFds, dummyPipeFds + 2), Return(0)));
	EXPECT_CALL(m_processOps, forkProcess).WillOnce(DoAll(SetArgReferee<0>(dummyPid), Return(0)));
	EXPECT_CALL(m_epollWrapper, addEvent).WillOnce(Return(false));

	m_connection.m_buffer.assign(
		"POST /cgi-bin/upperCase.sh HTTP/1.1\r\nHost:example.com\r\n\r\nContent-Length:12\r\n\r\nThis is body");

	handleCompleteRequestHeader(m_server, m_dummyFd, m_connection);

	EXPECT_EQ(m_connection.m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_connection.m_status, Connection::BuildResponse);
}
