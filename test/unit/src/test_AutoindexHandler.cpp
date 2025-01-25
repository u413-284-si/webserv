#include "gmock/gmock.h"
#include <gtest/gtest.h>

#include <stdexcept>

#include "MockFileSystemOps.hpp"

#include "AutoindexHandler.hpp"

TEST(AutoindexHandler, OpenDirectoyThrow)
{
	MockFileSystemOps fileSystemOps;
	AutoindexHandler autoindexHandler(fileSystemOps);

	EXPECT_CALL(fileSystemOps, openDirectory).WillOnce(testing::Throw(std::runtime_error("openDirectory failed")));

	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/", "/test");
	EXPECT_EQ(autoindex, "");
}

TEST(AutoindexHandler, ReadDirectoryThrow)
{
	MockFileSystemOps fileSystemOps;
	AutoindexHandler autoindexHandler(fileSystemOps);

	EXPECT_CALL(fileSystemOps, openDirectory).WillOnce(testing::Return((DIR*)1));
	EXPECT_CALL(fileSystemOps, readDirectory).WillOnce(testing::Throw(std::runtime_error("readDirectory failed")));
	EXPECT_CALL(fileSystemOps, closeDirectory).WillOnce(testing::Return(0));

	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/", "/test");
	EXPECT_EQ(autoindex, "");
}
