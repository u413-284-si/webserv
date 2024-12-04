#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "MockFileSystemPolicy.hpp"

#include "PostRequestHandler.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class PostRequestHandlerTest : public ::testing::Test {
protected:
	PostRequestHandlerTest() {

		fakeStat.st_size = 1024;
		fakeStat.st_mtime = 777;

		ON_CALL(fileSystemPolicy, writeToFile)
		.WillByDefault(testing::Return());
		ON_CALL(fileSystemPolicy, getFileStat)
		.WillByDefault(testing::Return(fakeStat));
	 }
	~PostRequestHandlerTest() override { }

	std::string path = "/workspaces/webserv/test/";
	std::string content = "Hello, World!";
	struct stat fakeStat = {};
    
	NiceMock<MockFileSystemPolicy> fileSystemPolicy;
};


TEST_F(PostRequestHandlerTest, NonExistingFile)
{
	// Arrange
	PostRequestHandler postRequestHandler(fileSystemPolicy);

	EXPECT_CALL(fileSystemPolicy, isExistingFile)
		.WillOnce(testing::Return(false));
	
	// Act
	postRequestHandler.execute(path, content);

	// Assert
	EXPECT_EQ(postRequestHandler.getResponse().str(), "{\n\"message\": \"File created successfully\",\n\"file\": \"" + path + "\",\n\"file_size\": " + std::to_string(fakeStat.st_size) + ",\n\"last_modified\": \"" + webutils::getLocaltimeString(fakeStat.st_mtime, "%Y-%m-%d %H:%M:%S") + "\",\n\"status\": \"created\"\n}\n");
}

TEST_F(PostRequestHandlerTest, ExistingFile)
{
	// Arrange
	PostRequestHandler postRequestHandler(fileSystemPolicy);

	EXPECT_CALL(fileSystemPolicy, isExistingFile)
		.WillOnce(testing::Return(true));
	
	// Act
	postRequestHandler.execute(path, content);

	// Assert
	EXPECT_EQ(postRequestHandler.getResponse().str(), "{\n\"message\": \"Data appended successfully\",\n\"file\": \"" + path + "\",\n\"file_size\": " + std::to_string(fakeStat.st_size) + ",\n\"last_modified\": \"" + webutils::getLocaltimeString(fakeStat.st_mtime, "%Y-%m-%d %H:%M:%S") + "\",\n\"status\": \"updated\"\n}\n");
}

TEST_F(PostRequestHandlerTest, GetFileStatThrow)
{
	// Arrange
	PostRequestHandler postRequestHandler(fileSystemPolicy);
	std::string errorMessage = "stat(): getFileStat failed";

	EXPECT_CALL(fileSystemPolicy, getFileStat)
		.WillOnce(testing::Throw(std::runtime_error(errorMessage)));
	
	// Act
	postRequestHandler.execute(path, content);

	// Assert
	EXPECT_EQ(postRequestHandler.getResponse().str(), "");
}
