#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "MockFileSystemPolicy.hpp"
#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"

using ::testing::Return;
using ::testing::Throw;

class ResponseBodyHandlerTest : public ::testing::Test {
	protected:
	ResponseBodyHandlerTest() { }
	~ResponseBodyHandlerTest() override { }

	const int m_dummyFd = 10;
	ConfigFile m_configFile = createDummyConfig();
	Socket m_serverSock = {
		.host = m_configFile.servers[0].host,
		.port = m_configFile.servers[0].port
	};

	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);
	std::string m_responseBody;
	MockFileSystemPolicy m_fileSystemPolicy;
	ResponseBodyHandler m_responseBodyHandler = ResponseBodyHandler(m_connection, m_responseBody, m_fileSystemPolicy);

	HTTPRequest& m_request = m_connection.m_request;
};

TEST_F(ResponseBodyHandlerTest, IndexCreated)
{
	EXPECT_CALL(m_fileSystemPolicy, openDirectory)
	.Times(1);
	EXPECT_CALL(m_fileSystemPolicy, readDirectory);
	EXPECT_CALL(m_fileSystemPolicy, closeDirectory)
	.Times(1);

	m_request.targetResource = "/proc/self/";
	m_request.httpStatus = StatusOK;
	m_request.hasAutoindex = true;
	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/proc/self/autoindex.html");
}

TEST_F(ResponseBodyHandlerTest, DirectoryThrow)
{
	EXPECT_CALL(m_fileSystemPolicy, openDirectory)
	.Times(2)
	.WillOnce(Throw(std::runtime_error("openDirectory failed")))
	.WillOnce(Return(nullptr));
	EXPECT_CALL(m_fileSystemPolicy, readDirectory)
	.WillOnce(Throw(std::runtime_error("readDirectory failed")));
	EXPECT_CALL(m_fileSystemPolicy, closeDirectory)
	.Times(1);

	m_request.targetResource = "/proc/self/";
	m_request.httpStatus = StatusOK;
	m_request.hasAutoindex = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);

	m_request.httpStatus = StatusOK;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
}

TEST_F(ResponseBodyHandlerTest, ErrorPage)
{
	m_request.targetResource = "/proc/self/";
	m_request.httpStatus = StatusForbidden;
	m_request.hasAutoindex = false;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileNotOpened)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(Throw(std::runtime_error("openFile failed")));

	m_request.hasAutoindex = false;
	m_request.httpStatus = StatusOK;
	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileFound)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(Return("Hello World"));

	m_request.hasAutoindex = false;
	m_request.httpStatus = StatusOK;
	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_responseBody, "Hello World");
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPage)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(Return(FileSystemPolicy::FileRegular));
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(Return("error_page_content"));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_responseBody, "error_page_content");
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageStrangeType)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(Return(FileSystemPolicy::FileOther));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusInternalServerError));
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageOpenFails)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
	.WillOnce(Return(FileSystemPolicy::FileRegular));
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(Throw(std::runtime_error("openFile failed")));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusInternalServerError));
}

TEST(ResponseBodyHandler, getDefaultErrorPage)
{
	getDefaultErrorPage(StatusOK);
	getDefaultErrorPage(StatusMovedPermanently);
	getDefaultErrorPage(StatusPermanentRedirect);
	getDefaultErrorPage(StatusBadRequest);
	getDefaultErrorPage(StatusForbidden);
	getDefaultErrorPage(StatusNotFound);
	getDefaultErrorPage(StatusRequestEntityTooLarge);
	getDefaultErrorPage(StatusMethodNotAllowed);
	getDefaultErrorPage(StatusRequestTimeout);
	getDefaultErrorPage(StatusRequestHeaderFieldsTooLarge);
	getDefaultErrorPage(StatusInternalServerError);
	getDefaultErrorPage(StatusMethodNotImplemented);
	getDefaultErrorPage(StatusNonSupportedVersion);
}
