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

// TEST(PostRequestHandler, ReadDirectoryThrow)
// {
// 	MockFileSystemPolicy fileSystemPolicy;
// 	PostRequestHandler postRequestHandler(fileSystemPolicy);

// 	EXPECT_CALL(fileSystemPolicy, openDirectory)
// 	.WillOnce(testing::Return((DIR*)1));
// 	EXPECT_CALL(fileSystemPolicy, readDirectory)
// 	.WillOnce(testing::Throw(std::runtime_error("readDirectory failed")));
// 	EXPECT_CALL(fileSystemPolicy, closeDirectory)
// 	.WillOnce(testing::Return(0));

// 	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/");
// 	EXPECT_EQ(autoindex, "");
// }
