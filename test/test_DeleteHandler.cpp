#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "MockFileSystemPolicy.hpp"

#include "DeleteHandler.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class DeleteHandlerTest : public ::testing::Test {
protected:
	DeleteHandlerTest()
	{
		ON_CALL(m_fileSystemPolicy, deleteFile).WillByDefault(Return());
		ON_CALL(m_fileSystemPolicy, deleteDirectory).WillByDefault(Return());
	}
	~DeleteHandlerTest() override { }

	std::string m_path = "/workspaces/webserv/test/";
	statusCode m_statusCode = StatusOK;

	NiceMock<MockFileSystemPolicy> m_fileSystemPolicy;
	DeleteHandler m_deleteHandler = DeleteHandler(m_fileSystemPolicy);
};

TEST_F(DeleteHandlerTest, DeleteFile)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemPolicy::FileRegular));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody,
		"{\n\"message\": \"File deleted successfully\",\n\"file\": \"" + m_path
			+ "\"\n}\n");
}

TEST_F(DeleteHandlerTest, DeleteDirectory)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemPolicy::FileDirectory));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody,
		"{\n\"message\": \"Directory deleted successfully\",\n\"directory\": \"" + m_path
			+ "\"\n}\n");
}

TEST_F(DeleteHandlerTest, FileNotExist)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemPolicy::FileNotExist));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusNotFound);
}

TEST_F(DeleteHandlerTest, FileOther)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemPolicy::FileOther));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusInternalServerError);
}

TEST_F(DeleteHandlerTest, Forbidden)
{
	// Arrange
	std::string errorMessage = "remove(): Permission denied";
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemPolicy::FileRegular));
	EXPECT_CALL(m_fileSystemPolicy, deleteFile).WillOnce(testing::Throw(std::runtime_error(errorMessage)));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusForbidden);
}
