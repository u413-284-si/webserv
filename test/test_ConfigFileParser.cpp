#include "ConfigFileParser.hpp"
#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class InvalidConfigFileTests : public ::testing::Test {
protected:
	ConfigFileParser m_configFileParser;
};

class ValidConfigFileTests : public ::testing::Test {
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
 * 4. File contains invalid directive (including server and location)
 * 5. File contains missing semicolon
 * 6. Listen directive contains invalid ip address
 * 7. Listen directive contains invalid port
 * 8. Root directive has no root path
 * 9. Root directive has multiple root paths
 * 10. Invalid directives outside of server block
 */

TEST_F(InvalidConfigFileTests, FileCouldNotBeOpened)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("invalid_file");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Failed to open config file", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileIsEmpty)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/empty_file.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Config file is empty", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsMissingBracket)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/missing_bracket.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Open bracket(s) in config file", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsTooManyBrackets)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/too_many_brackets.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Open bracket(s) in config file", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsInvalidServerDirective)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/invalid_server_directive.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid server directive", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsInvalidLocationDirective)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/invalid_location_directive.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid location directive", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsMissingSemicolon)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/missing_semicolon.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Unexpected '}'", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooHighIpAddress)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_ip_high.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid ip address", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsIpAddressWithMissingDot)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_ip_missing_dot.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid ip address", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooHighPort)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_port_high.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid port", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsTooLowPort)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_port_low.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid port", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, RootDirectiveContainsNoPath)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/root_no_path.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'root' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, RootDirectiveContainsMultipleRootPaths)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/root_multiple_paths.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one root path", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, InvalidDirectivesOutsideOfServerBlock)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/invalid_directive_outside_server.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid directive", e.what());
				throw;
			}
		},
		std::runtime_error);
}

/**
 * @brief Tests for a valid file
 *
 * Checks if the following configuration are seen as valid:
 * 1. A standard valid file
 * 2. File does not contain the http block
 * 3. File does not contain any server block
 * 4. Several directives on one line
 * 5. Listen directive contains only ip
 * 6. Listen directive contains only port
 * 7. Listen contains ip and port
 * 8. Listen contains only localhost
 * 9. Listen contains localhost and port
 * 10. Bracket under server directive
 * 11. Bracket under location directive
 * 12. Whitespaces between server directive and opening bracket
 * 13. Directive and opening bracket on the same line
 * 14. Directive and closing bracket on the same line
 * 15. Directive and closing bracket on the same line under server directive
 */

TEST_F(ValidConfigFileTests, ValidFile)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/valid_config.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("80", configFile.servers[0].port);
	EXPECT_EQ("/var/www/html", configFile.servers[0].root);
}

TEST_F(InvalidConfigFileTests, FileMissesHtppBlock)
{
	ConfigFile configFile = m_configFileParser.parseConfigFile("config_files/missing_http_block.conf");
	EXPECT_EQ(true, configFile.servers.empty());

	EpollWrapper epollWrapper(10, -1);
	SocketPolicy socketPolicy;
	ProcessOps processOps;

	Server server(configFile, epollWrapper, socketPolicy, processOps);

	initVirtualServers(server, 10, server.getServerConfigs());
	std::map<int, Socket> virtualServers = server.getVirtualServers();
	EXPECT_EQ(0, virtualServers.size());
}

TEST_F(InvalidConfigFileTests, FileMissesServerBlock)
{
	ConfigFile configFile = m_configFileParser.parseConfigFile("config_files/missing_server_block.conf");
	EXPECT_EQ(true, configFile.servers.empty());

	EpollWrapper epollWrapper(10, -1);
	SocketPolicy socketPolicy;
	ProcessOps processOps;

	Server server(configFile, epollWrapper, socketPolicy, processOps);

	initVirtualServers(server, 10, server.getServerConfigs());
	std::map<int, Socket> virtualServers = server.getVirtualServers();
	EXPECT_EQ(0, virtualServers.size());
}

TEST_F(ValidConfigFileTests, FileContainsSeveralDirectivesOnOneLine)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/several_directives_on_one_line.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("80", configFile.servers[0].port);
	EXPECT_EQ("/var/www/html", configFile.servers[0].root);
}

TEST_F(ValidConfigFileTests, ListenContainsOnlyIp)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/listen_only_ip.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
}

TEST_F(ValidConfigFileTests, ListenContainsOnlyPort)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/listen_only_port.conf"));
	EXPECT_EQ("80", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, ListenContainsIpAndPort)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/listen_ip_and_port.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("80", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, ListenContainsOnlyLocalhost)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/listen_only_localhost.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
}

TEST_F(ValidConfigFileTests, ListenContainsLocalhostAndPort)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/listen_localhost_and_port.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("9090", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, BracketUnderServerDirective)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/bracket_under_server.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
	EXPECT_EQ("127.0.0.1", configFile.servers[1].host);
	EXPECT_EQ("9090", configFile.servers[1].port);
}

TEST_F(ValidConfigFileTests, BracketUnderLocationDirective)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/bracket_under_location.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, WhiteSpacesBetweenServerDirectiveAndBracket)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/whitespaces_between_server_bracket.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, DirectiveAndOpeningBracketOnSameLine)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/directive_open_bracket_same_line.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, DirectiveAndClosingBracketOnSameLine)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/directive_close_bracket_same_line.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, DirectiveAndClosingBracketOnSameLineUnderServerDirective)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile
		= m_configFileParser.parseConfigFile("config_files/directive_close_open_bracket_same_line_under_server.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("8080", configFile.servers[0].port);
}

TEST_F(ValidConfigFileTests, LocationPath)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/location_path.conf"));
	EXPECT_EQ(3, configFile.servers[0].locations.size());
	EXPECT_EQ("/", configFile.servers[0].locations[0].path);
	EXPECT_EQ("/hello", configFile.servers[0].locations[1].path);
	EXPECT_EQ("/world", configFile.servers[0].locations[2].path);
}
