#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "MockFileSystemPolicy.hpp"

#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"

class ResponseBodyHandlerTest : public ::testing::Test {
	protected:
	ResponseBodyHandlerTest() : m_responseBodyHandler(m_response, m_fileSystemPolicy) { }
	~ResponseBodyHandlerTest() override { }

	HTTPResponse m_response;
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

	m_response.targetResource = "/proc/self/";
	m_response.status = StatusOK;
	m_response.isAutoindex = true;
	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.status, StatusOK);
	EXPECT_EQ(m_response.targetResource, "/proc/self/autoindex.html");
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

	m_response.targetResource = "/proc/self/";
	m_response.status = StatusOK;
	m_response.isAutoindex = true;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.status, StatusInternalServerError);

	m_response.status = StatusOK;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.status, StatusInternalServerError);
}

TEST_F(ResponseBodyHandlerTest, ErrorPage)
{
	m_response.targetResource = "/proc/self/";
	m_response.status = StatusForbidden;
	m_response.isAutoindex = false;

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.body, webutils::getDefaultErrorPage(m_response.status));
}

TEST_F(ResponseBodyHandlerTest, FileNotOpened)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(testing::Throw(std::runtime_error("openFile failed")));

	m_response.isAutoindex = false;
	m_response.status = StatusOK;
	m_response.method = MethodGet;
	m_response.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.status, StatusInternalServerError);
	EXPECT_EQ(m_response.body, webutils::getDefaultErrorPage(m_response.status));
}

TEST_F(ResponseBodyHandlerTest, FileFound)
{
	EXPECT_CALL(m_fileSystemPolicy, getFileContents)
	.WillOnce(testing::Return("Hello World"));

	m_response.isAutoindex = false;
	m_response.status = StatusOK;
	m_response.method = MethodGet;
	m_response.targetResource = "/proc/self/cmdline";

	m_responseBodyHandler.execute();
	EXPECT_EQ(m_response.status, StatusOK);
	EXPECT_EQ(m_response.body, "Hello World");
}
