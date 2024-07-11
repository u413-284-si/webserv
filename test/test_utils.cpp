#include <gtest/gtest.h>

#include "utils.hpp"

TEST(getFileExtension, NoExtension)
{
	EXPECT_EQ(utils::getFileExtension("file"), "");
	EXPECT_EQ(utils::getFileExtension("some/path/to/file"), "");
	EXPECT_EQ(utils::getFileExtension("a/path/with.inbetween/to/file"), "");
}

TEST(getFileExtension, Extension)
{
	EXPECT_EQ(utils::getFileExtension("file.txt"), "txt");
	EXPECT_EQ(utils::getFileExtension("a/path/to/file.txt"), "txt");
	EXPECT_EQ(utils::getFileExtension("a/path/with.inbetween/to/file.txt"), "txt");
}
