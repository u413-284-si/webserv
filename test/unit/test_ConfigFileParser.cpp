#include "ConfigFileParser.hpp"
#include "StatusCode.hpp"
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
 * Checks if the following configurations are seen as invalid:
 * 1. File could not be opened (because the path is wrong, such a file does not exist etc.)
 * 2. File is empty
 * 3. File does not contain the http block
 * 4. File does not contain any server block
 * 5. File contains open brackets (including missing brackets and too many brackets)
 * 6. File contains invalid directive (including server and location)
 * 7. File contains missing semicolon
 * 8. Server contains duplicate location
 * 9. Server contains duplicate location with default path
 * 10. Listen directive contains invalid ip address
 * 11. Listen directive contains invalid port
 * 12. Listen contains invalid amount of parameters with a host and port
 * 13. Listen contains invalid amount of parameters with an ip address
 * 14. Listen contains invalid amount of parameters with a port
 * 15. Listen contains invalid amount of parameters with localhost as host
 * 16. Listen directive contains no value
 * 17. Root directive contains no root path
 * 18. Root directive contains multiple root paths
 * 19. Root directive contains no slash at the beginning
 * 20. Alias directive contains no alias path
 * 21. Alias directive contains multiple alias paths
 * 22. Alias directive contains no slash at the beginning
 * 23. Root and alias are defined in the same location
 * 24. Max body size directive contains no number
 * 25. Max body size directive contains invalid char within number
 * 26. Max body size directive contains invalid unit char
 * 27. Max body size directive contains invalid unit length
 * 28. Max body size contains invalid amount of parameters
 * 29. Max body size contains number which causes an overflow
 * 30. Max body size contains unit which causes an overflow
 * 31. Max body size directive contains no value
 * 32. Autoindex directive contains invalid value
 * 33. Autoindex contains invalid amount of parameters
 * 34. Autoindex contains no value
 * 35. Allow methods directive contains invalid value
 * 36. Allow methods contains no value
 * 37. Error page contains invalid amount of parameters
 * 38. Error page contains invalid error code in between valid error codes (eg. 303 is in between 301 and 308. 303 is
 still invalid because it is not implemented)
 * 39. Error page contains invalid error code lower as the lowest error code
 * 40. Error page contains invalid error code higher as the highest error code
 * 41. Error page path contains no slash at the beginning
 * 42. Error page path contains no value
 * 43. Error page contains no value
 * 44. CGI extension contains no dot at beginning
 * 45. CGI extension contains multiple extensions
 * 46. CGI extension contains multiple dots at the beginning
 * 47. CGI extension contains multiple dots in between
 * 48. CGI extension contains no value
 * 49. CGI path contains no slash at the beginning
 * 50. CGI path contains multiple paths
 * 51. CGI path contains no value
 * 52. CGI index contains no value
 * 53. Return contains invalid code in between valid codes
 * 54. Return contains invalid code lower as the lowest code
 * 55. Return contains invalid code higher as the highest code
 * 56. Return contains invalid url
 * 57. Return contains invalid amount of parameters
 * 58. Return contains invalid amount of parameters with double quotes
 * 59. Return contains unclosed double quote
 * 60. Return contains no value
 * 61. Invalid directives outside of server block
 * 62. Several server names
 * 63. Server name contains no value
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

TEST_F(InvalidConfigFileTests, FileContainsMissingHttpBlock)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/missing_http_block.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Missing http block", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, FileContainsMissingServerBlock)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/missing_server_block.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Missing server block(s)", e.what());
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

TEST_F(InvalidConfigFileTests, ServerContainsDuplicateLocation)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/duplicate_location.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Duplicate location", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ServerContainsDuplicateLocationDefaultPath)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/duplicate_location_default_path.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Duplicate location", e.what());
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

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsInvalidAmountOfParametersWithHostAndPorWithHostAndPort)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_amount_parameters_host_and_port.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for listen", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsInvalidAmountOfParametersWithIp)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_amount_parameters_ip.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for listen", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsInvalidAmountOfParametersWithPort)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_amount_parameters_port.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for listen", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsInvalidAmountOfParametersWithLocalhost)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_invalid_amount_parameters_localhost.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for listen", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ListenDirectiveContainsNoValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/listen_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'listen' directive has no value", e.what());
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

TEST_F(InvalidConfigFileTests, RootDirectiveContainsNoSlash)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/root_no_slash.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Root path does not start with a slash", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AliasDirectiveContainsNoPath)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/alias_no_path.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'alias' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AliasDirectiveContainsMultipleRootPaths)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/alias_multiple_paths.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one alias path", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AliasDirectiveContainsNoSlash)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/alias_no_slash.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Alias path does not start with a slash", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, RootAndAliasInSameLocation)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/location_root_and_alias.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Defining root and alias in the same location block is not allowed", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsNoNumber)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_no_number.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsInvalidCharWithinNumber)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_invalid_char_within_number.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsInvalidUnitChar)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_invalid_unit_char.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size unit", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsInvalidUnitLenght)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_invalid_unit_length.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size unit", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsInvalidAmountOfParameters)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_invalid_amount_parameters.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for client_max_body_size", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsNumberWhichCausesOverflow)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_overflow_number.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size number: Overflow", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsUnitWhichCausesOverflow)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_overflow_unit.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid client_max_body_size unit: Overflow", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, MaxBodySizeContainsNoValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/max_body_size_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'client_max_body_size' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AutoIndexContainsInvalidValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/autoindex_invalid.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid autoindex value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AutoIndexContainsInvalidAmountOfParameters)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/autoindex_invalid_amount_parameters.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for autoindex", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AutoIndexContainsNoValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/autoindex_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'autoindex' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AllowMethodsContainsInvalidValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/allow_methods_invalid.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid allow_methods value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, AllowMethodsContainsNoValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/allow_methods_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'allow_methods' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsInvalidAmountOfParameters)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_invalid_amount_parameters.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for error_page", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsInvalidErrorCodeInBetweenValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile(
					"config_files/error_page_invalid_error_code_in_between_valid_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid error code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsInvalidErrorCodeLowerThanValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_invalid_error_code_lower_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid error code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsInvalidErrorCodeHigherThanValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_invalid_error_code_higher_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid error code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsEmptyErrorPagePath)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_empty_error_page_path.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("error_page directive path has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPagePathContainsNoSlashAtBeginning)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_path_no_slash.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Error page path does not start with a slash", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ErrorPageContainsEmptyValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/error_page_empty_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'error_page' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIExtensionContainsNoDotAtBeginning)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_extension_no_dot.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid CGI extension", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIExtensionContainsMultipleExtensions)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_extension_multiple_extensions.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one CGI extension", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIExtensionContainsMultipleDotsBeginning)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_extension_multiple_dots_beginning.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one dot in CGI extension", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIExtensionContainsMultipleDotsInbetween)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_extension_multiple_dots_inbetween.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one dot in CGI extension", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIExtensionContainsEmptyValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_extension_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'cgi_ext' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIPathContainsPathWithoutSlashAtBeginning)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_path_no_slash.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("CGI path does not start with a slash", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIPathContainsMultiplePaths)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_path_multiple_paths.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one CGI path", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, CGIPathContainsEmptyValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/cgi_path_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'cgi_path' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidCodeInBetweenValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_code_in_between_valid_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid return code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidCodeLowerThanValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_code_lower_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid return code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidCodeHigherThanValidRange)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_code_higher_range.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid return code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidUrl)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_url.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid return code", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidAmountOfParameters)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_amount_parameters.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for return", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsInvalidAmountOfParametersWithDoubleQuotes)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_invalid_amount_parameters_quotes.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Invalid amount of parameters for return", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsUnclosedQuote)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_unclosed_quote.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("Open double quotes", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ReturnContainsEmptyValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/return_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'return' directive has no value", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, IndexContainsEmptyValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/index_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'index' directive has no value", e.what());
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

TEST_F(InvalidConfigFileTests, SeveralServerNames)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/invalid_server_name_several_server_names.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("More than one server name", e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(InvalidConfigFileTests, ServerNameContainsNoValue)
{
	EXPECT_THROW(
		{
			try {
				m_configFileParser.parseConfigFile("config_files/server_name_no_value.conf");
			} catch (const std::exception& e) {
				EXPECT_STREQ("'server_name' directive has no value", e.what());
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
 * 2. Several directives on one line
 * 3. Server name contains one name
 * 4. Server name contains empty string
 * 5. Listen contains only ip
 * 6. Listen contains only port
 * 7. Listen contains ip and port
 * 8. Listen contains only localhost
 * 9. Listen contains localhost and port
 * 10. Alias path contains one path
 * 11. Max body size contains only number
 * 12. Max body size contains number and k unit
 * 13. Max body size contains number and K unit
 * 14. Max body size contains number and m unit
 * 15. Max body size contains number and M unit
 * 16. Max body size contains number and g unit
 * 17. Max body size contains number and G unit
 * 18. Autoindex contains on
 * 19. Autoindex contains off
 * 20. Autoindex contains valid mix of lowercase and uppercase
 * 21. Allow methods contains GET
 * 22. Allow methods contains get
 * 23. Allow methods contains POST
 * 24. Allow methods contains post
 * 25. Allow methods contains DELETE
 * 26. Allow methods contains delete
 * 27. Allow methods contains GET, POST and DELETE
 * 28. Allow methods contains GET, POST and DELETE with mixed cases
 * 29. Error page contains multiple error codes and error page paths
 * 30. CGI extension contains one extension
 * 31. CGI path contains one path
 * 32. Index contains multiple indices
 * 33. Return contains code and text with double quotes
 * 34. Return contains code and text without double quotes
 * 35. Return contains code and url
 * 36. Return contains only code
 * 37. Return contains only http url
 * 38. Return contains only https url
 * 39. Bracket under server directive
 * 40. Bracket under location directive
 * 41. Whitespaces between server directive and opening bracket
 * 42. Directive and opening bracket on the same line
 * 43. Directive and closing bracket on the same line
 * 44. Directive and closing bracket on the same line under server directive
 * 45. Location path
 * 46. Inheritance of the server directives root, max_body_size and error_page to location
 */

TEST_F(ValidConfigFileTests, ValidFile)
{
	ConfigFile configFile;

	// server block
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/valid_config.conf"));
	EXPECT_EQ("127.0.0.1", configFile.servers[0].host);
	EXPECT_EQ("80", configFile.servers[0].port);
	EXPECT_EQ("/var/www/html", configFile.servers[0].root);
	EXPECT_EQ(2097152, configFile.servers[0].maxBodySize);
	EXPECT_EQ("/error/404.html", configFile.servers[0].errorPage[StatusNotFound]);

	// location '/' block
	EXPECT_EQ("/var/www/html", configFile.servers[0].locations[0].root);
	EXPECT_EQ(1073741824, configFile.servers[0].locations[0].maxBodySize);
	EXPECT_EQ("index.html", configFile.servers[0].locations[0].indices[0]);
	EXPECT_EQ("default.html", configFile.servers[0].locations[0].indices[1]);

	// location '/list' block
	EXPECT_EQ("/var/www", configFile.servers[0].locations[1].root);
	EXPECT_EQ(true, configFile.servers[0].locations[1].hasAutoindex);

	// location '/upload' block
	EXPECT_EQ("/var/www", configFile.servers[0].locations[2].root);
	EXPECT_EQ(true, configFile.servers[0].locations[2].allowMethods[MethodPost]);
	EXPECT_EQ("/error/403.html", configFile.servers[0].locations[2].errorPage[StatusForbidden]);

	// location '/cgi' block
	EXPECT_EQ("/var/www", configFile.servers[0].locations[3].root);
	EXPECT_EQ(".py", configFile.servers[0].locations[3].cgiExt);
	EXPECT_EQ("/usr/bin/python3", configFile.servers[0].locations[3].cgiPath);

	// location '/images' block
	EXPECT_EQ("/var/www/images", configFile.servers[0].locations[4].alias);
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

TEST_F(ValidConfigFileTests, ServerNameContainsOneName)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/server_name_one_name.conf"));
	EXPECT_EQ("greatestWebsite.com", configFile.servers[0].serverName);
}

TEST_F(ValidConfigFileTests, ServerNameContainsEmptyString)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/server_name_empty_server_name.conf"));
	EXPECT_EQ("", configFile.servers[0].serverName);
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

TEST_F(ValidConfigFileTests, AliasContainsOnlyPath)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/alias_one_path.conf"));
	EXPECT_EQ("/var/www/images", configFile.servers[0].locations[4].alias);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsOnlyNumber)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_only_number.conf"));
	EXPECT_EQ(10, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitKLower)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_k_lower.conf"));
	EXPECT_EQ(5120, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitKUpper)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_k_upper.conf"));
	EXPECT_EQ(5120, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitMLower)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_m_lower.conf"));
	EXPECT_EQ(15728640, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitMUpper)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_m_upper.conf"));
	EXPECT_EQ(15728640, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitGLower)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_g_lower.conf"));
	EXPECT_EQ(2147483648, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, MaxBodySizeContainsAndUnitGUpper)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/max_body_size_number_and_unit_g_upper.conf"));
	EXPECT_EQ(2147483648, configFile.servers[0].maxBodySize);
}

TEST_F(ValidConfigFileTests, AutoindexContainsOn)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/autoindex_on.conf"));
	EXPECT_EQ(true, configFile.servers[0].locations[0].hasAutoindex);
}

TEST_F(ValidConfigFileTests, AutoindexContainsOff)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/autoindex_off.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].hasAutoindex);
}

TEST_F(ValidConfigFileTests, AutoindexContainsValidLowerCaseUpperCaseMix)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/autoindex_lowercase_uppercase.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].hasAutoindex);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsGetLowercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_get_lowercase.conf"));
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsGetUppercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_get_uppercase.conf"));
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsPostLowercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_post_lowercase.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsPostUppercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_post_uppercase.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsDeleteLowercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_delete_lowercase.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsDeleteUppercase)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_delete_uppercase.conf"));
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(false, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsGetPostDelete)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_get_post_delete.conf"));
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, AllowMethodsContainsGetPostDeleteMixedCases)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/allow_methods_get_post_delete_mixed_cases.conf"));
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[0]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[1]);
	EXPECT_EQ(true, configFile.servers[0].locations[0].allowMethods[2]);
}

TEST_F(ValidConfigFileTests, ErrorPageContainsMultipleErrorCodesAndErrorPagePaths)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/error_page_multiple_errors_pages.conf"));
	EXPECT_EQ("/error/404.html", configFile.servers[0].errorPage[StatusNotFound]);
	EXPECT_EQ("/error/405.html", configFile.servers[0].errorPage[StatusMethodNotAllowed]);
	EXPECT_EQ("/error/400.html", configFile.servers[0].locations[0].errorPage[StatusBadRequest]);
	EXPECT_EQ("/error/403.html", configFile.servers[0].locations[0].errorPage[StatusForbidden]);
}

TEST_F(ValidConfigFileTests, CGIExtensionContainsOneExtensions)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/cgi_extension_python.conf"));
	EXPECT_EQ(".py", configFile.servers[0].locations[0].cgiExt);
}

TEST_F(ValidConfigFileTests, CGIPathContainsOnePath)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/cgi_path_python.conf"));
	EXPECT_EQ("/usr/bin/python3", configFile.servers[0].locations[0].cgiPath);
}

TEST_F(ValidConfigFileTests, IndexContainsMultipleIndices)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/index_multiple_indices.conf"));
	EXPECT_EQ("index.html", configFile.servers[0].locations[0].indices[0]);
	EXPECT_EQ("default.html", configFile.servers[0].locations[0].indices[1]);
}

TEST_F(ValidConfigFileTests, ReturnContainsCodeAndTextWithQuotes)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/return_code_and_text_quotes.conf"));
	EXPECT_EQ(StatusOK, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("42 is the answer!", configFile.servers[0].locations[1].returns.second);
}

TEST_F(ValidConfigFileTests, ReturnContainsCodeAndTextWithoutQuotes)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(
		configFile = m_configFileParser.parseConfigFile("config_files/return_code_and_text_without_quotes.conf"));
	EXPECT_EQ(StatusOK, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("hello", configFile.servers[0].locations[1].returns.second);
}

TEST_F(ValidConfigFileTests, ReturnContainsCodeAndUrl)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/return_code_and_url.conf"));
	EXPECT_EQ(StatusOK, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("/secret/index.html", configFile.servers[0].locations[1].returns.second);
}

TEST_F(ValidConfigFileTests, ReturnContainsOnlyCode)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/return_only_code.conf"));
	EXPECT_EQ(StatusNotFound, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("", configFile.servers[0].locations[1].returns.second);
}

TEST_F(ValidConfigFileTests, ReturnContainsOnlyUrlHttp)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/return_only_url_http.conf"));
	EXPECT_EQ(StatusFound, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("http://google.com", configFile.servers[0].locations[1].returns.second);
}

TEST_F(ValidConfigFileTests, ReturnContainsOnlyUrlHttps)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/return_only_url_https.conf"));
	EXPECT_EQ(StatusFound, configFile.servers[0].locations[1].returns.first);
	EXPECT_EQ("https://google.com", configFile.servers[0].locations[1].returns.second);
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

TEST_F(ValidConfigFileTests, DirectiveInheritance)
{
	ConfigFile configFile;
	EXPECT_NO_THROW(configFile = m_configFileParser.parseConfigFile("config_files/directive_inheritance.conf"));
	EXPECT_EQ("/var/www/html", configFile.servers[0].locations[0].root);
}
