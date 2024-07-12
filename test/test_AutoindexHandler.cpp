#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MockFileSystemPolicy.hpp"

#include "AutoindexHandler.hpp"

TEST(AutoindexHandler, Autoindex)
{
	MockFileSystemPolicy fileSystemPolicy;
	AutoindexHandler autoindexHandler(fileSystemPolicy);
	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/");
	EXPECT_EQ(autoindex, "..");
}