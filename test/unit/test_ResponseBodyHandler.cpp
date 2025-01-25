#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockFileSystemOps.hpp"
#include "ResponseBodyHandler.hpp"

ConfigFile createTestConfigfile();

using ::testing::Return;
using ::testing::Throw;

class ResponseBodyHandlerTest : public ::testing::Test {
protected:
	ResponseBodyHandlerTest()
	{
		m_configFile.servers[0].locations[0].errorPage[StatusNotFound] = "/error";
		m_configFile.servers[0].locations[0].errorPage[StatusForbidden] = "/error_with_return";
		m_configFile.servers[0].locations[0].errorPage[StatusBadRequest] = "/error_with_empty_return";

		Location location2;
		location2.path = "/error";

		Location location3;
		location3.path = "/error_with_return";
		location3.returns = std::make_pair(StatusMovedPermanently, "Return Message");

		Location location4;
		location4.path = "/error_with_empty_return";
		location4.returns = std::make_pair(StatusMovedPermanently, "");

		m_configFile.servers[0].locations.push_back(location2);
		m_configFile.servers[0].locations.push_back(location3);
		m_configFile.servers[0].locations.push_back(location4);

		m_connection.location = m_configFile.servers[0].locations.begin();
	}
	~ResponseBodyHandlerTest() override { }

	const int m_dummyFd = 10;
	Socket m_serverSock = { .host = "127.0.0.1", .port = "8080" };
	ConfigFile m_configFile = createTestConfigfile();
	Connection m_connection = Connection(m_serverSock, Socket(), m_dummyFd, m_configFile.servers);

	HTTPRequest& m_request = m_connection.m_request;

	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
	MockFileSystemOps m_fileSystemOps;
	ResponseBodyHandler m_responseBodyHandler = ResponseBodyHandler(m_connection, m_responseBody, m_responseHeaders, m_fileSystemOps);
};

TEST_F(ResponseBodyHandlerTest, IndexCreated)
{
	EXPECT_CALL(m_fileSystemOps, openDirectory).Times(1);
	EXPECT_CALL(m_fileSystemOps, readDirectory);
	EXPECT_CALL(m_fileSystemOps, closeDirectory).Times(1);

	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/";
	m_request.hasAutoindex = true;
	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/proc/self/autoindex.html");
}

TEST_F(ResponseBodyHandlerTest, OpenDirectoryThrow)
{
	EXPECT_CALL(m_fileSystemOps, openDirectory)
	.WillOnce(Throw(std::runtime_error("openDirectory failed")));

	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/";
	m_request.hasAutoindex = true;

	m_responseBodyHandler.execute();

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
}

TEST_F(ResponseBodyHandlerTest, ReadDirectoryThrow)
{
	EXPECT_CALL(m_fileSystemOps, openDirectory)
	.WillOnce(Return(nullptr));
	EXPECT_CALL(m_fileSystemOps, readDirectory)
	.WillOnce(Throw(std::runtime_error("readDirectory failed")));
	EXPECT_CALL(m_fileSystemOps, closeDirectory)
	.Times(1);

	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/";
	m_request.hasAutoindex = true;

	m_responseBodyHandler.execute();

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
}

TEST_F(ResponseBodyHandlerTest, ErrorPage)
{
	m_request.targetResource = "/proc/self/";
	m_request.httpStatus = StatusInternalServerError;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileNotOpened)
{
	EXPECT_CALL(m_fileSystemOps, getFileContents).WillOnce(Throw(std::runtime_error("openFile failed")));

	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileFound)
{
	EXPECT_CALL(m_fileSystemOps, getFileContents).WillOnce(Return("Hello World"));

	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_responseBody, "Hello World");
}

TEST_F(ResponseBodyHandlerTest, FileWentMissing)
{
	EXPECT_CALL(m_fileSystemOps, getFileContents)
	.WillOnce(Throw(FileSystemOps::FileNotFoundException("getFileContents failed")))
	.WillOnce(Return("error_page_content"));
	EXPECT_CALL(m_fileSystemOps, checkFileType)
	.WillOnce(Return(FileSystemOps::FileRegular));

	m_request.hasAutoindex = false;
	m_request.httpStatus = StatusOK;
	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_responseBody, "error_page_content");
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPage)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_fileSystemOps, getFileContents).WillOnce(Return("error_page_content"));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_responseBody, "error_page_content");
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageStrangeType)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileOther));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusForbidden));
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageOpenFails)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_fileSystemOps, getFileContents).WillOnce(Throw(std::runtime_error("openFile failed")));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusInternalServerError));
}

TEST_F(ResponseBodyHandlerTest, ReturnAsSimpleRedirect)
{
	m_request.httpStatus = StatusPermanentRedirect;
	m_request.targetResource = "/new_location";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusPermanentRedirect);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusPermanentRedirect));
}

TEST_F(ResponseBodyHandlerTest, ReturnWithNonErrorCodeAndNoContent)
{
	m_request.httpStatus = StatusOK;
	m_request.targetResource = "";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_responseBody, "");
}

TEST_F(ResponseBodyHandlerTest, ReturnWithNoRedirectAndCustomContent)
{
	m_request.httpStatus = StatusNonSupportedVersion;
	m_request.targetResource = "Return Message";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusNonSupportedVersion);
	EXPECT_EQ(m_responseBody, "Return Message");
}

TEST_F(ResponseBodyHandlerTest, ReturnWithErrorCodeAndNoContentFindsDefaultErrorPage)
{
	m_request.httpStatus = StatusMethodNotAllowed;
	m_request.targetResource = "";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusMethodNotAllowed);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusMethodNotAllowed));
}

TEST_F(ResponseBodyHandlerTest, ReturnWithErrorCodeAndNoContentFindsCustomErrorPageWithReturnNoContent)
{
	m_request.httpStatus = StatusBadRequest;
	m_request.targetResource = "";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusBadRequest);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusBadRequest));
}

TEST_F(ResponseBodyHandlerTest, ReturnWithErrorCodeAndNoContentFindsCustomErrorPageWithReturnWithContent)
{
	m_request.httpStatus = StatusForbidden;
	m_request.targetResource = "";
	m_request.hasReturn = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_responseBody, "Return Message");
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageWentMissing)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
	.WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_fileSystemOps, getFileContents)
	.WillOnce(Throw(FileSystemOps::FileNotFoundException("File not found")));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusNotFound));
}

TEST_F(ResponseBodyHandlerTest, CustomErrorPageNoPermission)
{
	EXPECT_CALL(m_fileSystemOps, checkFileType)
	.WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_fileSystemOps, getFileContents)
	.WillOnce(Throw(FileSystemOps::NoPermissionException("No permission")));

	m_request.httpStatus = StatusNotFound;
	m_request.targetResource = "/not_existing";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_responseBody, getDefaultErrorPage(StatusForbidden));
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
