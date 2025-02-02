#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>

#include "MockFileSystemOps.hpp"

#include "DeleteHandler.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class DeleteHandlerTest : public ::testing::Test {
protected:
	DeleteHandlerTest()
	{
		ON_CALL(m_fileSystemPolicy, deleteFile).WillByDefault(Return());
	}
	~DeleteHandlerTest() override { }

	std::string m_path = "/workspaces/webserv/html/test/";
	statusCode m_statusCode = StatusOK;

	NiceMock<MockFileSystemOps> m_fileSystemPolicy;
	DeleteHandler m_deleteHandler = DeleteHandler(m_fileSystemPolicy);
};

TEST_F(DeleteHandlerTest, DeleteFile)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody,
		"{\n\"message\": \"File deleted successfully\",\n\"file\": \"" + m_path
			+ "\"\n}\n");
}

// FIXME: reactivate after eval
// TEST_F(DeleteHandlerTest, DeleteDirectory)
// {
// 	// Arrange
// 	std::filesystem::create_directory(m_path);
// 	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemOps::FileDirectory));
// 	EXPECT_CALL(m_fileSystemPolicy, openDirectory).WillOnce(Return(nullptr));
// 	EXPECT_CALL(m_fileSystemPolicy, readDirectory).WillOnce(Return(nullptr));

// 	// Act
// 	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

// 	// Assert
// 	EXPECT_EQ(responseBody,
// 		"{\n\"message\": \"Directory deleted successfully\",\n\"directory\": \"" + m_path
// 			+ "\"\n}\n");
// }

TEST_F(DeleteHandlerTest, FileNotFound)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemOps::FileNotFound));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusNotFound);
}

TEST_F(DeleteHandlerTest, FileOther)
{
	// Arrange
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemOps::FileOther));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusForbidden);
}

TEST_F(DeleteHandlerTest, Forbidden)
{
	// Arrange
	std::string errorMessage = "remove(): Permission denied";
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(Return(FileSystemOps::FileRegular));
	EXPECT_CALL(m_fileSystemPolicy, deleteFile).WillOnce(testing::Throw(FileSystemOps::NoPermissionException(errorMessage)));

	// Act
	std::string responseBody = m_deleteHandler.execute(m_path, m_statusCode);

	// Assert
	EXPECT_EQ(responseBody, "");
	EXPECT_EQ(m_statusCode, StatusForbidden);
}
