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

		m_fakeStat.st_size = 1024;
		m_fakeStat.st_mtime = 777;

		ON_CALL(m_fileSystemPolicy, deleteFile).WillByDefault(Return());
		ON_CALL(m_fileSystemPolicy, getFileStat).WillByDefault(Return(m_fakeStat));
	}
	~DeleteHandlerTest() override { }

	std::string m_path = "/workspaces/webserv/test/";
	statusCode m_statusCode = StatusOK;
	struct stat m_fakeStat = {};

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

// TEST_F(DeleteHandlerTest, ExistingFile)
// {
// 	// Arrange
// 	EXPECT_CALL(m_fileSystemPolicy, isExistingFile).WillOnce(Return(true));

// 	// Act
// 	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content);

// 	// Assert
// 	EXPECT_EQ(responseBody,
// 		"{\n\"message\": \"Data appended successfully\",\n\"file\": \"" + m_path
// 			+ "\",\n\"file_size\": " + std::to_string(m_fakeStat.st_size) + ",\n\"last_modified\": \""
// 			+ webutils::getLocaltimeString(m_fakeStat.st_mtime, "%Y-%m-%d %H:%M:%S")
// 			+ "\",\n\"status\": \"updated\"\n}\n");
// }

// TEST_F(DeleteHandlerTest, GetFileStatThrow)
// {
// 	// Arrange
// 	std::string errorMessage = "stat(): getFileStat failed";

// 	EXPECT_CALL(m_fileSystemPolicy, getFileStat).WillOnce(testing::Throw(std::runtime_error(errorMessage)));

// 	// Act
// 	std::string responseBody = m_fileWriteHandler.execute(m_path, m_content);

// 	// Assert
// 	EXPECT_EQ(responseBody, "");
// }
