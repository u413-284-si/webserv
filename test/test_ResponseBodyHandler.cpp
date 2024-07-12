#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "MockFileSystemPolicy.hpp"

#include "ResponseBodyHandler.hpp"
#include "StatusCode.hpp"

class ResponseBodyHandlerTest : public ::testing::Test {
	protected:
	ResponseBodyHandlerTest() { }
	~ResponseBodyHandlerTest() override { }

	HTTPResponse m_response;
	MockFileSystemPolicy m_fileSystemPolicy;
};

TEST_F(ResponseBodyHandlerTest, IndexCreated)
{
	EXPECT_CALL(m_fileSystemPolicy, openDirectory)
	.Times(1);
	EXPECT_CALL(m_fileSystemPolicy, readDirectory);
	EXPECT_CALL(m_fileSystemPolicy, closeDirectory)
	.Times(1);

	ResponseBodyHandler responseBodyHandler(m_fileSystemPolicy);

	m_response.targetResource = "/proc/self/";
	m_response.autoindex = true;
	m_response = responseBodyHandler.execute(m_response);
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

	ResponseBodyHandler responseBodyHandler(m_fileSystemPolicy);

	m_response.targetResource = "/proc/self/";
	m_response.autoindex = true;

	m_response = responseBodyHandler.execute(m_response);
	EXPECT_EQ(m_response.status, StatusInternalServerError);

	m_response = responseBodyHandler.execute(m_response);
	EXPECT_EQ(m_response.status, StatusInternalServerError);
}
