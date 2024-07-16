#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include "ConfigFileParser.hpp"

TEST(InvalidConfigFile, FileCouldNotBeOpened)
{
	ConfigFileParser configFileParser;
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("invalid_file");}, testing::ThrowsMessage<std::runtime_error>("Failed to open config file"));
}

TEST(InvalidConfigFile, FileIsEmpty)
{
	ConfigFileParser configFileParser;
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/empty_file.conf");}, testing::ThrowsMessage<std::runtime_error>("Config file is empty"));
}

TEST(InvalidConfigFile, FileContainsOpenBrackets)
{
	ConfigFileParser configFileParser;
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/open_brackets.conf");}, testing::ThrowsMessage<std::runtime_error>("Open bracket(s) in config file"));
}
