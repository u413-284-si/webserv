#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ConfigFileParser.hpp"

class InvalidConfigFileTests : public ::testing::Test
{
	protected:
		ConfigFileParser configFileParser;
};

TEST_F(InvalidConfigFileTests, FileCouldNotBeOpened)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("invalid_file");}, testing::ThrowsMessage<std::runtime_error>("Failed to open config file"));
}

TEST_F(InvalidConfigFileTests, FileIsEmpty)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/empty_file.conf");}, testing::ThrowsMessage<std::runtime_error>("Config file is empty"));
}

TEST_F(InvalidConfigFileTests, FileContainsMissingBracket)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/missing_bracket.conf");}, testing::ThrowsMessage<std::runtime_error>("Open bracket(s) in config file"));
}

TEST_F(InvalidConfigFileTests, FileContainsTooManyBrackets)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/too_many_brackets.conf");}, testing::ThrowsMessage<std::runtime_error>("Open bracket(s) in config file"));
}