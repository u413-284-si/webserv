#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ConfigFileParser.hpp"

class InvalidConfigFileTests : public ::testing::Test
{
	protected:
		ConfigFileParser configFileParser;
};

class ValidConfigFileTests : public ::testing::Test
{
	protected:
		ConfigFileParser configFileParser;
};

/**
 * @brief Tests for an invalid file
 * 
 * Checks if the following configuration are seen as invalid:
 * 1. File could not be opened (because the path is wrong, such a file does not exist etc.)
 * 2. File is empty
 * 3. File has open brackets (including missing brackets and too many brackets)
 * 4. File is missing the http block
 * 5. File is missing server block
 * 6. File contains invalid directive (including server)
 * @todo FIXME: Check for invalid location directive
 * @todo FIXME: Check for missing semicolon
 * @todo FIXME: Check for invalid ip address
 * @todo FIXME: Check for invalid port
 */


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

TEST_F(InvalidConfigFileTests, FileMissesHtppBlock)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/missing_http_block.conf");}, testing::ThrowsMessage<std::runtime_error>("Config file does not start with 'http {'"));
}

TEST_F(InvalidConfigFileTests, FileMissesServerBlock)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/missing_server_block.conf");}, testing::ThrowsMessage<std::runtime_error>("No server(s) in config file"));
}

TEST_F(InvalidConfigFileTests, FileContainsInvalidServerDirective)
{
	EXPECT_THAT([&]() {configFileParser.parseConfigFile("config_files/invalid_server_directive.conf");}, testing::ThrowsMessage<std::runtime_error>("Invalid server directive"));
}

/**
 * @brief Tests for a valid file
 * 
 * Checks if the following configuration are seen as valid:
 * 1. Listen directive contains only ip
 * 2. Listen directive contains only port
 * 3. Listen contains ip and port
 */

TEST_F(ValidConfigFileTests, ListenContainsOnlyIp)
{
	EXPECT_NO_THROW([&]() {configFileParser.parseConfigFile("config_files/listen_only_ip.conf");});
}

TEST_F(ValidConfigFileTests, ListenContainsOnlyPort)
{
	EXPECT_NO_THROW([&]() {configFileParser.parseConfigFile("config_files/listen_only_port.conf");});
}

TEST_F(ValidConfigFileTests, ListenContainsIpAndPort)
{
	EXPECT_NO_THROW([&]() {configFileParser.parseConfigFile("config_files/listen_ip_and_port.conf");});
}
