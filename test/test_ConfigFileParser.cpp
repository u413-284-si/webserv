#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ConfigFileParser.hpp"

class InvalidConfigFileTests : public ::testing::Test
{
	protected:
		ConfigFileParser m_configFileParser;
};

class ValidConfigFileTests : public ::testing::Test
{
	protected:
		ConfigFileParser m_configFileParser;
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
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("invalid_file");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Failed to open config file", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileIsEmpty)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/empty_file.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Config file is empty", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsMissingBracket)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/missing_bracket.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Open bracket(s) in config file", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsTooManyBrackets)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/too_many_brackets.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Open bracket(s) in config file", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileMissesHtppBlock)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/missing_http_block.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Config file does not start with 'http {'", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileMissesServerBlock)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/missing_server_block.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("No server(s) in config file", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsInvalidServerDirective)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/invalid_server_directive.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid server directive", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsMissingSemicolon)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/missing_semicolon.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Semicolon missing", e.what());
			throw;
		}
	}, std::runtime_error);
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
	EXPECT_NO_THROW([&]() {m_configFileParser.parseConfigFile("config_files/listen_only_ip.conf");});
}

TEST_F(ValidConfigFileTests, ListenContainsOnlyPort)
{
	EXPECT_NO_THROW([&]() {m_configFileParser.parseConfigFile("config_files/listen_only_port.conf");});
}

TEST_F(ValidConfigFileTests, ListenContainsIpAndPort)
{
	EXPECT_NO_THROW([&]() {m_configFileParser.parseConfigFile("config_files/listen_ip_and_port.conf");});
}
