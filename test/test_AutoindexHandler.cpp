#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "FileHandlerMock.hpp"

#include "AutoindexHandler.hpp"

TEST(AutoindexHandler, Autoindex)
{
	FileHandler fileHandler;
	AutoindexHandler autoindexHandler(fileHandler);
	std::string autoindex = autoindexHandler.execute("/workspaces/webserv/test/");
	EXPECT_EQ(autoindex, "..");
}