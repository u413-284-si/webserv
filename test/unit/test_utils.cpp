#include <gtest/gtest.h>

#include "utilities.hpp"

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

TEST(replaceAlias, AliasFound)
{
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "/path", "/replacement"), "/replacement/to/somewhere");
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "/path/to/somewhere", "/replacement"), "/replacement");
}

TEST(replaceAlias, AliasNotFound)
{
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "/alias", "/replacement"), "");
}

TEST(replaceAlias, AliasNotFoundAtBeginning)
{
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "/somewhere", "/replacement"), "");
}

TEST(replaceAlias, InputIsEmpty)
{
	EXPECT_EQ(webutils::replaceAlias("", "/path", "/replacement"), "");
}

TEST(replaceAlias, AliasIsEmpty)
{
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "", "/replacement"), "/replacement/path/to/somewhere");
}

TEST(replaceAlias, ReplacementIsEmpty)
{
	EXPECT_EQ(webutils::replaceAlias("/path/to/somewhere", "/path", ""), "/to/somewhere");
}

TEST(replaceAlias, MultipleInstancesOfAlias)
{
	EXPECT_EQ(webutils::replaceAlias("/alias/alias/alias/alias/", "/alias", "/replacement"), "/replacement/alias/alias/alias/");
}
