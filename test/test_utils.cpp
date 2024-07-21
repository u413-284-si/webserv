#include <gtest/gtest.h>

#include "utils.hpp"

TEST(getFileExtension, NoExtension)
{
	EXPECT_EQ(webutils::getFileExtension("file"), "");
	EXPECT_EQ(webutils::getFileExtension("some/path/to/file"), "");
	EXPECT_EQ(webutils::getFileExtension("a/path/with.inbetween/to/file"), "");
}

TEST(getFileExtension, Extension)
{
	EXPECT_EQ(webutils::getFileExtension("file.txt"), "txt");
	EXPECT_EQ(webutils::getFileExtension("a/path/to/file.txt"), "txt");
	EXPECT_EQ(webutils::getFileExtension("a/path/with.inbetween/to/file.txt"), "txt");
}
