#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "MockFileSystemPolicy.hpp"

#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"

class ResponseBodyHandlerTest : public ::testing::Test {
	protected:
	ResponseBodyHandlerTest() : m_responseBodyHandler(m_request, m_responseBody, m_fileSystemPolicy) { }
	~ResponseBodyHandlerTest() override { }

	HTTPRequest m_request;
	std::string m_responseBody;
	MockFileSystemPolicy m_fileSystemPolicy;
	ResponseBodyHandler m_responseBodyHandler;
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
	.WillOnce(testing::Throw(std::runtime_error("openDirectory failed")))
	.WillOnce(testing::Return(nullptr));
	EXPECT_CALL(m_fileSystemPolicy, readDirectory)
	.WillOnce(testing::Throw(std::runtime_error("readDirectory failed")));
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
	EXPECT_EQ(m_responseBody, webutils::getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileNotOpened)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(testing::Throw(std::runtime_error("openFile failed")));

	m_request.hasAutoindex = false;
	m_request.httpStatus = StatusOK;
	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_responseBody, webutils::getDefaultErrorPage(m_request.httpStatus));
}

TEST_F(ResponseBodyHandlerTest, FileFound)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(testing::Return("Hello World"));

	m_request.hasAutoindex = false;
	m_request.httpStatus = StatusOK;
	m_request.method = MethodGet;
	m_request.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_responseBody, "Hello World");
}
