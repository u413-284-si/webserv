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
 * 6. File contains invalid directive (including server and location)
 * 7. File contains missing semicolon
 * 8. Listen directive contains invalid ip address
 * 9. Listen directive contains invalid port
 * 10. Root directive has no root path
 * 11. Root directive has multiple root paths
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

TEST_F(InvalidConfigFileTests, FileContainsInvalidLocationDirective)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/invalid_location_directive.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid location directive", e.what());
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

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooHighIpAddress)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/listen_invalid_ip_high.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid ip address", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsIpAddressWithMissingDot)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/listen_invalid_ip_missing_dot.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid ip address", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooHighPort)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/listen_invalid_port_high.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid port", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooLowPort)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/listen_invalid_port_low.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("Invalid port", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, RootDirectiveContainsNoPath)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/root_no_path.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("'root' directive has no value", e.what());
			throw;
		}
	}, std::runtime_error);
}

TEST_F(InvalidConfigFileTests, RootDirectiveContainsMultipleRootPaths)
{
	EXPECT_THROW(
	{
		try
		{
			m_configFileParser.parseConfigFile("config_files/root_multiple_paths.conf");
		}
		catch (const std::exception& e)
		{
			EXPECT_STREQ("More than one root path", e.what());
			throw;
		}
	}, std::runtime_error);
}


/**
 * @brief Tests for a valid file
 * 
 * Checks if the following configuration are seen as valid:
 * 1. A standard valid file
 * 2. Listen directive contains only ip
 * 3. Listen directive contains only port
 * 4. Listen contains ip and port
 */

TEST_F(ValidConfigFileTests, ValidFile)

{
	EXPECT_NO_THROW([&]() {m_configFileParser.parseConfigFile("config_files/valid_config.conf");});
}
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
