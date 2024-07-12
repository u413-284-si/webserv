#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>

#include "MockFileSystemPolicy.hpp"

#include "AutoindexHandler.hpp"

TEST(AutoindexHandler, OpenDirectoyThrow)
{
	MockFileSystemPolicy fileSystemPolicy;
	AutoindexHandler autoindexHandler(fileSystemPolicy);

	EXPECT_CALL(fileSystemPolicy, openDirectory)
	.WillOnce(testing::Throw(std::runtime_error("openDirectory failed")));

	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/");
	EXPECT_EQ(autoindex, "");
}

TEST(AutoindexHandler, ReadDirectoryThrow)
{
	MockFileSystemPolicy fileSystemPolicy;
	AutoindexHandler autoindexHandler(fileSystemPolicy);

	DIR* dir = opendir("/proc/self/");

	EXPECT_CALL(fileSystemPolicy, openDirectory)
	.WillOnce(testing::Return(dir));
	EXPECT_CALL(fileSystemPolicy, readDirectory)
	.WillOnce(testing::Throw(std::runtime_error("readDirectory failed")));
	EXPECT_CALL(fileSystemPolicy, closeDirectory)
	.WillOnce(testing::Return(0));

	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/");
	EXPECT_EQ(autoindex, "");
}
