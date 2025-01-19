#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "MockFileSystemPolicy.hpp"

#include "FileWriteHandler.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class FileWriteHandlerTest : public ::testing::Test {
protected:
	FileWriteHandlerTest()
	{

		m_fakeStat.st_size = 1024;
		m_fakeStat.st_mtime = 777;

		ON_CALL(m_fileSystemPolicy, writeToFile).WillByDefault(Return());
		ON_CALL(m_fileSystemPolicy, getFileStat).WillByDefault(Return(m_fakeStat));
	}
	~FileWriteHandlerTest() override { }

	std::string m_path = "/workspaces/webserv/test/";
	std::string m_content = "Hello, World!";
	struct stat m_fakeStat = {};
	statusCode m_httpStatus = StatusOK;

	NiceMock<MockFileSystemPolicy> m_fileSystemPolicy;
	FileWriteHandler m_fileWriteHandler = FileWriteHandler(m_fileSystemPolicy);
};

TEST_F(FileWriteHandlerTest, NonExistingFile)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, isExistingFile).WillOnce(Return(false));

	// Act
	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content, m_httpStatus);

	// Assert
	EXPECT_EQ(responseBody,
		"{\n\"message\": \"File created successfully\",\n\"file\": \"" + m_path
			+ "\",\n\"file_size\": " + std::to_string(m_fakeStat.st_size) + ",\n\"last_modified\": \""
			+ webutils::getLocaltimeString(m_fakeStat.st_mtime, "%Y-%m-%d %H:%M:%S")
			+ "\",\n\"status\": \"created\"\n}\n");
	EXPECT_EQ(m_httpStatus, StatusCreated);
}

TEST_F(FileWriteHandlerTest, ExistingFile)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, isExistingFile).WillOnce(Return(true));

	// Act
	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content, m_httpStatus);

	// Assert
	EXPECT_EQ(responseBody,
		"{\n\"message\": \"Data appended successfully\",\n\"file\": \"" + m_path
			+ "\",\n\"file_size\": " + std::to_string(m_fakeStat.st_size) + ",\n\"last_modified\": \""
			+ webutils::getLocaltimeString(m_fakeStat.st_mtime, "%Y-%m-%d %H:%M:%S")
			+ "\",\n\"status\": \"updated\"\n}\n");
	EXPECT_EQ(m_httpStatus, StatusOK);
}

TEST_F(FileWriteHandlerTest, GetFileStatThrow)
{
	// Arrange
	std::string errorMessage = "stat(): getFileStat failed";

	EXPECT_CALL(m_fileSystemPolicy, getFileStat).WillOnce(testing::Throw(std::runtime_error(errorMessage)));

	// Act
	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content, m_httpStatus);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_httpStatus, StatusInternalServerError);
}

TEST_F(FileWriteHandlerTest, NoPermission)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, isExistingFile)
	.WillOnce(testing::Throw(FileSystemPolicy::NoPermissionException("No permission")));

	// Act
	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content, m_httpStatus);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_httpStatus, StatusForbidden);
}

TEST_F(FileWriteHandlerTest, FileNotFound)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, writeToFile)
	.WillOnce(testing::Throw(FileSystemPolicy::FileNotFoundException("File not found")));

	// Act
	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content, m_httpStatus);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_httpStatus, StatusNotFound);
}
