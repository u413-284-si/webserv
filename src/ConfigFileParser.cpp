#include "ConfigFileParser.hpp"

const char* const ConfigFileParser::s_whitespace = " \t\n\v\f\r";
const char* const ConfigFileParser::s_number = "0123456789";

/**
 * @brief PUBLIC Construct a new ConfigFileParser:: ConfigFileParser object.
 */
ConfigFileParser::ConfigFileParser(void)
	: m_configFile()
	, m_configFileIndex(0)
	, m_contentIndex(0)
	, m_serverIndex(0)
	, m_locationIndex(0)
	, m_serverRootCount(0)
	, m_locationRootCount(0)
	, m_isDefaultLocationDefined(false)
{
	const char* validServerDirectiveNames[]
		= { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root" };
	const int validServerDirectiveNamesSize = sizeof(validServerDirectiveNames) / sizeof(validServerDirectiveNames[0]);

	const char* validLocationDirectiveNames[] = { "root", "alias", "index", "cgi_ext", "cgi_path",
		"client_max_body_size", "autoindex", "error_page", "allow_methods", "location", "return" };
	const int validLocationDirectiveNamesSize
		= sizeof(validLocationDirectiveNames) / sizeof(validLocationDirectiveNames[0]);

	m_validServerDirectives = std::vector<std::string>(validServerDirectiveNames,
		validServerDirectiveNames
			+ validServerDirectiveNamesSize); // NOLINT: no good alternative for pointer arithmetic
	m_validLocationDirectives = std::vector<std::string>(validLocationDirectiveNames,
		validLocationDirectiveNames
			+ validLocationDirectiveNamesSize); // NOLINT: no good alternative for pointer arithmetic
}

/**
 * @brief Parses the config file into a ConfigFile object and returns this object
 *
 * The function checks also if the config file:
 * 1. Can be opened
 * 2. Is not empty
 * 3. Does not contain open brackets
 * 4. Is not missing the http block
 * 5. Does not contain invalid directives
 * 6. Is not missing server block(s)
 *
 * @param configFilePath Path to the config file
 * @return const ConfigFile& Created ConfigFile object
 */
const ConfigFile& ConfigFileParser::parseConfigFile(const std::string& configFilePath)
{
	std::ifstream fileStream(configFilePath.c_str());
	std::stringstream bufferStream;

	if (!fileStream.is_open())
		throw std::runtime_error(ERR_CONFIG_FILE_OPEN_FAILED);
	if (fileStream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error(ERR_CONFIG_FILE_EMPTY);

	bufferStream << fileStream.rdbuf();
	m_configFileContent = bufferStream.str();
	fileStream.close();

	if (isBracketOpen())
		throw std::runtime_error(ERR_OPEN_BRACKET_IN_CONFIG_FILE);

	if (!isValidBlockBegin(HttpBlock))
		throw std::runtime_error(ERR_MISSING_HTTP_BLOCK);

	skipBlockBegin(HttpBlock);

	while (m_configFileContent[m_configFileIndex] != '}') {
		if (isValidBlockBegin(ServerBlock))
			readServerBlock();
		else if (std::isspace(m_configFileContent[m_configFileIndex]) == 0)
			throw std::runtime_error(ERR_INVALID_DIRECTIVE);
		m_configFileIndex++;
	}

	if (m_serverBlocksConfig.empty())
		throw std::runtime_error(ERR_MISSING_SERVER_BLOCK);

	for (std::vector<ServerBlockConfig>::const_iterator serverIt = m_serverBlocksConfig.begin();
		 serverIt != m_serverBlocksConfig.end(); serverIt++) {
		processServerContent(*serverIt);
		m_serverIndex++;
	}

	if (isLocationDuplicate())
		throw std::runtime_error(ERR_DUPLICATE_LOCATION);

	return m_configFile;
}

/*********************/
/* CHECKER FUNCTIONS */
/*********************/

/**
 * @brief Looks for a keyword in the m_configFileContent (eg. http, server, location)
 *
 * @param keyword The keyword to look for
 * @return true If the keyword is found
 * @return false If the keyword is not found
 */
bool ConfigFileParser::isKeyword(const std::string& keyword, size_t startIndex) const
{
	std::string string = m_configFileContent.substr(startIndex, keyword.length());
	return string == keyword;
}

/**
 * @brief Checks if the beginning of a block is valid
 *
 * @param block The block to check
 * @return true If the beginning of the block is valid
 * @return false If the beginning of the block is not valid
 */
bool ConfigFileParser::isValidBlockBegin(Block block)
{
	size_t index = m_configFileIndex;

	while (std::isspace(m_configFileContent[index]) != 0)
		index++;

	if (block == HttpBlock && !isKeyword(convertBlockToString(HttpBlock), index))
		return false;
	if (block == ServerBlock && !isKeyword(convertBlockToString(ServerBlock), index))
		return false;
	if (block == LocationBlock && !isKeyword(convertBlockToString(LocationBlock), index))
		return false;

	index += convertBlockToString(block).length();

	while (std::isspace(m_configFileContent[index]) != 0)
		index++;

	if (block == LocationBlock) {
		if (m_configFileContent[index] == '{')
			return false;
		skipLocationBlockPath(index);
	}

	return m_configFileContent[index] == '{';
}

/**
 * @brief Checks if there are no open brackets in the config file
 *
 * If a opening bracket is found, it is pushed onto the brackets stack
 * If a closing bracket is found, it is popped from the brackets stack
 *
 * At the end the stack should be empty. If not, there are open brackets
 *
 * @return true If there is minimum one open bracket
 * @return false If there are no open bracket
 */
bool ConfigFileParser::isBracketOpen(void)
{
	std::stack<char> brackets;

	for (std::string::const_iterator it = m_configFileContent.begin(); it != m_configFileContent.end(); ++it) {
		if (*it == '{')
			brackets.push('{');
		else if (*it == '}' && brackets.empty())
			return true;
		else if (*it == '}')
			brackets.pop();
	}
	return !brackets.empty();
}

/**
 * @brief Checks if a semicolon at the end of the content of a block is missing
 *
 * @param content The content of a block
 * @return true If the semicolon is missing
 * @return false If the semicolon is not missing
 */
bool ConfigFileParser::isSemicolonMissing(const std::string& content) const
{
	size_t nonWhitepaceIndex = content.find_last_not_of(s_whitespace);
	return content[nonWhitepaceIndex] != ';';
}

/**
 * @brief Checks if the directive is valid for the given block
 *
 * @param directive The directive to check
 * @param block The block which surounds the directive
 * @return true When the directive is valid
 * @return false When the directive is invalid
 */

bool ConfigFileParser::isDirectiveValid(const std::string& directive, Block block) const
{
	if (block == ServerBlock) {
		if (std::find(m_validServerDirectives.begin(), m_validServerDirectives.end(), directive)
			== m_validServerDirectives.end())
			return false;
	} else if (block == LocationBlock) {
		if (std::find(m_validLocationDirectives.begin(), m_validLocationDirectives.end(), directive)
			== m_validLocationDirectives.end())
			return false;
	}
	return true;
}

/**
 * @brief Checks if there are duplicate locations
 *
 * @return true If there are duplicate locations
 * @return false If there are no duplicate locations
 */
bool ConfigFileParser::isLocationDuplicate(void) const
{
	for (std::vector<ConfigServer>::const_iterator serverIt = m_configFile.servers.begin();
		 serverIt != m_configFile.servers.end(); serverIt++) {
		std::set<std::string> paths;
		for (std::vector<Location>::const_iterator locationIt = serverIt->locations.begin();
			 locationIt != serverIt->locations.end(); locationIt++) {
			if (paths.find(locationIt->path) != paths.end())
				return true;
			paths.insert(locationIt->path);
		}
	}
	return false;
}

/*********************/
/* READER FUNCTIONS */
/*********************/

/**
 * @brief Reads a server block into a struct ServerContent
 */
void ConfigFileParser::readServerBlock(void)
{
	skipBlockBegin(ServerBlock);

	ServerBlockConfig serverBlockConfig;
	size_t startIndex = m_configFileIndex;

	while (m_configFileContent[m_configFileIndex] != '}') {
		if (isKeyword(convertBlockToString(LocationBlock), m_configFileIndex)) {
			if (!isValidBlockBegin(LocationBlock))
				throw std::runtime_error(ERR_LOCATION_INVALID_BEGIN);
			serverBlockConfig.serverBlockContent
				+= m_configFileContent.substr(startIndex, m_configFileIndex - startIndex);
			readLocationBlock(serverBlockConfig);
			startIndex = m_configFileIndex;
		}
		m_configFileIndex++;
	}

	serverBlockConfig.serverBlockContent += m_configFileContent.substr(startIndex, m_configFileIndex - startIndex);

	m_configFileIndex++;
	m_serverBlocksConfig.push_back(serverBlockConfig);
}

/**
 * @brief Reads a location block into a struct LocationContent
 *
 * @param serverBlockConfig The associated struct ServerBlockConfig
 */
void ConfigFileParser::readLocationBlock(ServerBlockConfig& serverBlockConfig)
{
	std::string locationBlockContent;
	size_t startIndex = m_configFileIndex;

	while (m_configFileContent[m_configFileIndex] != '}')
		m_configFileIndex++;

	locationBlockContent = m_configFileContent.substr(startIndex, m_configFileIndex - startIndex);

	m_configFileIndex++;
	serverBlockConfig.locationBlocksContent.push_back(locationBlockContent);
}

/**
 * @brief Processes the content of a server block
 *
 * The content will be processed by reading it line by line (delimited by ';')
 * In this process the values of the directives will be read and stored
 * If there are location blocks, they will be processed as well
 *
 * If there is a semicolon missing, an exception will be thrown
 *
 * @param serverBlockConfig The config of the server block
 */
void ConfigFileParser::processServerContent(const ServerBlockConfig& serverBlockConfig)
{
	ConfigServer server;
	m_configFile.servers.push_back(server);
	m_locationIndex = 0;

	if (isSemicolonMissing(serverBlockConfig.serverBlockContent))
		throw std::runtime_error(ERR_SEMICOLON_MISSING);

	m_serverRootCount = 0;
	while (readAndTrimLine(serverBlockConfig.serverBlockContent, ';'))
		readServerConfigLine();

	m_isDefaultLocationDefined = false;
	for (std::vector<std::string>::const_iterator it = serverBlockConfig.locationBlocksContent.begin();
		 it != serverBlockConfig.locationBlocksContent.end(); ++it) {
		m_locationRootCount = 0;
		processLocationContent(*it);
	}
}

/**
 * @brief Processes the content of a location block
 *
 * The content will be processed by reading it line by line (delimited by ';')
 * In this process the values of the directives will be read and stored
 *
 * If the path of the location block is "/", the m_locationIndex will be set to 0 in readLocationBlockPath
 * This is necessary because the following functions need to store the parsed values in the default location.
 * To continue with the correct value of m_locationIndex the original value is stored in tmpIndex and will be used
 * if the location index is 0
 *
 * If root and alias are defined in the same location block, an exception will be thrown
 *
 * If no values are specified for the root, max_body_size, and error_page directives in a location block,
 * they inherit their corresponding values from the server block.
 *
 * If there is a semicolon missing, an exception will be thrown
 *
 * @param locationBlockContent The content of the location block
 */
void ConfigFileParser::processLocationContent(const std::string& locationBlockContent)
{
	size_t tmpIndex = m_locationIndex;
	readAndTrimLine(locationBlockContent, '{');
	readLocationBlockPath();

	if (isSemicolonMissing(locationBlockContent))
		throw std::runtime_error(ERR_SEMICOLON_MISSING);

	while (readAndTrimLine(locationBlockContent, ';'))
		readLocationConfigLine();

	ConfigServer& server = m_configFile.servers[m_serverIndex];
	Location& location = server.locations[m_locationIndex];

	if (location.root != "html" && !location.alias.empty())
		throw std::runtime_error(ERR_ROOT_AND_ALIAS_DEFINED);
	if (location.root == "html")
		location.root = server.root;
	else if (location.maxBodySize == constants::g_oneMegabyte)
		location.maxBodySize = server.maxBodySize;
	else if (location.errorPage.empty())
		location.errorPage = server.errorPage;

	if (m_locationIndex == 0)
		m_locationIndex = tmpIndex;
}

/**
 * @brief Reads the current line of the content, delimited by a provided char and removes leading and trailing
 * spaces. If the end of the content is reached, the function returns false, otherwise it returns true
 *
 * @param content The string from which the line should be read
 * @param delimiter The char which delimits the line
 * @return true If the end of the content is not reached
 * @return false If the end of the content is reached
 */
bool ConfigFileParser::readAndTrimLine(const std::string& content, char delimiter)
{
	size_t startIndex = m_contentIndex;

	while (content[m_contentIndex] != delimiter && m_contentIndex < content.size())
		m_contentIndex++;

	if (m_contentIndex == content.size()) {
		m_contentIndex = 0;

		m_currentLine = content.substr(startIndex, m_contentIndex - startIndex);

		m_currentLine = webutils::trimLeadingWhitespaces(m_currentLine);
		webutils::trimTrailingWhiteSpaces(m_currentLine);

		return false;
	}

	m_contentIndex++;
	m_currentLine = content.substr(startIndex, m_contentIndex - startIndex);

	m_currentLine = webutils::trimLeadingWhitespaces(m_currentLine);
	webutils::trimTrailingWhiteSpaces(m_currentLine);
	return true;
}

/**
 * @brief Reads the path of a location block
 *
 * If the path is "/", the m_locationIndex will be set to 0 because the default location has an index of 0
 * Therefore the following functions will store the values correctly in the default location
 * A boolean is set to indicate that the default location has been defined.
 * If there is also another location block with
 *
 * Otherwise a new location will be created, added to the locations vector and the m_locationIndex will be
 * incremented
 *
 */
void ConfigFileParser::readLocationBlockPath(void)
{
	const size_t startIndex = sizeof("location") - 1; // subtract zero terminator
	std::string pathWithLeadingWhitespaces = m_currentLine.substr(startIndex);
	std::string pathWithBracketAtEnd = webutils::trimLeadingWhitespaces(pathWithLeadingWhitespaces);

	size_t endIndex = 0;
	while (std::isspace(pathWithBracketAtEnd[endIndex]) == 0)
		endIndex++;

	std::string path = pathWithBracketAtEnd.substr(0, endIndex);
	if (path == "/" && !m_isDefaultLocationDefined) {
		m_locationIndex = 0;
		m_isDefaultLocationDefined = true;
	} else {
		Location location;
		m_configFile.servers[m_serverIndex].locations.push_back(location);
		m_locationIndex++;
	}
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].path = path;
}

/**
 * @brief Reads the root path
 *
 * The function checks if the root path is valid and reads it if that is the case.
 *
 * It makes sure that the path is valid in the following ways:
 * 1. There is only one root path
 * 2. There is a slash at the beginning of the path
 *
 * If at the end of the path is a slash, it removes it.
 *
 * For the case that a root directive was already read within the same server or location, a corresponding error is
 * thrown
 *
 * @param block The block which surounds the directive
 * @param rootPath The value of the directive root
 */
void ConfigFileParser::readRootPath(const Block& block, std::string rootPath)
{
	if (m_serverRootCount > 1)
		throw std::runtime_error(ERR_SERVER_MULTIPLE_ROOTS);
	if (m_locationRootCount > 1)
		throw std::runtime_error(ERR_LOCATION_MULTIPLE_ROOTS);

	if (rootPath.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_ROOT_PATHS);

	if (rootPath.at(0) != '/')
		throw std::runtime_error(ERR_ROOT_PATH_MISSING_SLASH);

	if (rootPath[rootPath.length() - 1] == '/')
		rootPath.erase(rootPath.end() - 1);

	if (block == ServerBlock)
		m_configFile.servers[m_serverIndex].root = rootPath;
	else if (block == LocationBlock)
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].root = rootPath;
}

/**
 * @brief Reads the alias path
 *
 * The function checks if the alias path is valid and reads it if that is the case.
 *
 * It makes sure that the path is valid in the following ways:
 * 1. There is only one alias path
 * 2. There is a slash at the beginning of the path
 *
 * @param aliasPath The value of the directive alias
 */
void ConfigFileParser::readAliasPath(const std::string& aliasPath)
{
	if (aliasPath.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_ALIAS_PATHS);

	if (aliasPath.at(0) != '/')
		throw std::runtime_error(ERR_ALIAS_PATH_MISSING_SLASH);

	m_configFile.servers[m_serverIndex].locations[m_locationIndex].alias = aliasPath;
}

/**
 * @brief Reads the server name
 *
 * The function checks if there is only one server name and reads it if that is the case
 * If an empty string is provided, the server name will be set to an empty string
 *
 * @param serverName The value of the directive server_name
 */

void ConfigFileParser::readServerName(const std::string& serverName)
{
	if (serverName.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_SERVER_NAMES);
	if (serverName == "\"\"")
		m_configFile.servers[m_serverIndex].serverName = "";
	else
		m_configFile.servers[m_serverIndex].serverName = serverName;
}

/**
 * @brief Reads the socket (inlcuding ip address and port)
 *
 * The function checks if the socket is valid and reads it if that is the case.
 *
 * At first the functions checks if there is a colon in the value of the directive.
 * If that is the case, the function checks and reads the ip address AND port.
 *
 * When there is no colon, the function checks if there is a dot in the value of the directive.
 * If that is the case, the function checks if the value equals "localhost".
 * When the value equals "localhost" the ip address "127.0.0.1" gets stored as host.
 * When the value does NOT equal "localhost" the value gets checked and stored as host.
 * Otherwise it checks and reads the port.
 *
 * @param value The value of the directive listen
 */
void ConfigFileParser::readListen(const std::string& value)
{
	const size_t colonIndex = value.find(':');
	const size_t endIndex = value.length();
	const size_t dot = value.find('.');

	if (colonIndex != std::string::npos) {
		std::string ipAddress = value.substr(0, colonIndex);
		if (!webutils::isIpAddressValid(ipAddress))
			throw std::runtime_error(ERR_INVALID_IP_ADDRESS);

		if (ipAddress == "localhost")
			m_configFile.servers[m_serverIndex].host = "127.0.0.1";
		else
			m_configFile.servers[m_serverIndex].host = ipAddress;

		std::string port = value.substr(colonIndex + 1, endIndex - colonIndex - 1);
		if (port.find_first_of(s_whitespace) != std::string::npos)
			throw std::runtime_error(ERR_INVALID_LISTEN_PARAMETERS);
		if (!webutils::isPortValid(port))
			throw std::runtime_error(ERR_INVALID_PORT);
		m_configFile.servers[m_serverIndex].port = port;
		return;
	}

	if (dot == std::string::npos) {
		std::string hostOrPort = value.substr(0, endIndex);
		if (hostOrPort.find_first_of(s_whitespace) != std::string::npos)
			throw std::runtime_error(ERR_INVALID_LISTEN_PARAMETERS);
		if (hostOrPort.empty())
			throw std::runtime_error(ERR_EMPTY_LISTEN_VALUE);
		if (hostOrPort == "localhost")
			m_configFile.servers[m_serverIndex].host = "127.0.0.1";
		else {
			if (!webutils::isPortValid(hostOrPort))
				throw std::runtime_error(ERR_INVALID_PORT);
			m_configFile.servers[m_serverIndex].port = hostOrPort;
		}
		return;
	}

	std::string ipAddress = value.substr(0, endIndex);
	if (ipAddress.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_INVALID_LISTEN_PARAMETERS);
	if (!webutils::isIpAddressValid(ipAddress))
		throw std::runtime_error(ERR_INVALID_IP_ADDRESS);

	m_configFile.servers[m_serverIndex].host = ipAddress;
}

/**
 * @brief Reads the max body size including the unit
 *
 * The function checks if the value of the directive is valid and reads it if that is the case.
 *
 * In general there can be two cases:
 * 1. The value is solely a number
 * 2. The value is a number followed by a unit
 *
 * At first it will be checked if there is a number as value
 * If that is the case the function will check if the number is valid and does not contain any other characters
 * Additionly it will be checked if the number itself causes an overflow
 *
 * When the checks for number are passed, the function will check if there is a unit
 * If that is the case the function will check if the unit is valid and
 * only consists of one character which is a valid unit
 * Valid units are: k (for kilobyte), m (for megabyte) and g (for gigabyte)
 *
 * Furthermore the function checks if the multiplication of the number with the unit causes an overflow
 *
 * @param block The block which surounds the directive
 * @param maxBodySize The value of the directive client_max_body_size
 */
void ConfigFileParser::readMaxBodySize(const Block& block, const std::string& maxBodySize)
{
	if (maxBodySize.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_PARAMETERS);

	const size_t lastNumberIndex = maxBodySize.find_last_of(s_number);
	if (lastNumberIndex == std::string::npos)
		throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_VALUE);

	const std::string number = maxBodySize.substr(0, lastNumberIndex + 1);
	if (number.find_first_not_of(s_number) != std::string::npos)
		throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_VALUE);

	errno = 0;
	size_t size = std::strtoul(number.c_str(), NULL, constants::g_decimalBase);
	if (errno == ERANGE)
		throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_NUMBER_OVERFLOW);

	if (lastNumberIndex != maxBodySize.size() - 1) {
		const std::string letter = maxBodySize.substr(lastNumberIndex + 1);
		if (letter.length() != 1)
			throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_UNIT);

		size_t unit = 1;
		switch (letter.at(0)) {
		case 'k':
		case 'K':
			unit = constants::g_oneKilobyte;
			break;
		case 'm':
		case 'M':
			unit = constants::g_oneMegabyte;
			break;
		case 'g':
		case 'G':
			unit = constants::g_oneGigabyte;
			break;
		default:
			throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_UNIT);
		}

		if (size > std::numeric_limits<size_t>::max() / unit)
			throw std::runtime_error(ERR_INVALID_MAX_BODY_SIZE_UNIT_OVERFLOW);

		size *= unit;
	}

	if (block == ServerBlock)
		m_configFile.servers[m_serverIndex].maxBodySize = size;
	else if (block == LocationBlock)
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].maxBodySize = size;
}

/**
 * @brief Reads the autoindex
 *
 * The function checks if the value of the directive is "on" or "off".
 * If that is the case the function will set the autoindex to true or false.
 * Otherwise it will throw an exception.
 *
 * @param autoindex The value of the directive autoindex
 */
void ConfigFileParser::readAutoIndex(const std::string& autoindex)
{
	if (autoindex.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_INVALID_AUTOINDEX_PARAMETERS);

	std::string lowercaseAutoindex = autoindex;
	webutils::lowercase(lowercaseAutoindex);

	if (lowercaseAutoindex == "on")
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].hasAutoindex = true;
	else if (lowercaseAutoindex == "off")
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].hasAutoindex = false;
	else
		throw std::runtime_error(ERR_INVALID_AUTOINDEX_VALUE);
}

/**
 * @brief Reads the allow_methods
 *
 * The function checks if the value of the directive is either "GET", "POST" or "DELETE".
 * If that is the case the function will set the corresponding allow method to true.
 * Otherwise it will throw an exception.
 *
 * Before setting the allow method to true the function will set all other allow methods to false
 *
 * @param allowMethods The value of the directive allow_methods
 */
void ConfigFileParser::readAllowMethods(const std::string& allowMethods)
{
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[0] = false;
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[1] = false;
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[2] = false;

	size_t index = 0;
	while (index < allowMethods.length()) {

		size_t methodStartIndex = index;
		size_t methodEndIndex = allowMethods.find_first_of(s_whitespace, index);

		std::string method = allowMethods.substr(methodStartIndex, methodEndIndex - methodStartIndex);
		webutils::lowercase(method);

		if (method == "get")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[0] = true;
		else if (method == "post")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[1] = true;
		else if (method == "delete")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[2] = true;
		else
			throw std::runtime_error(ERR_INVALID_ALLOW_METHODS);

		index = allowMethods.find_first_not_of(s_whitespace, methodEndIndex);
	}
}

/**
 * @brief Reads the error codes and corresponding error pages
 *
 * The function checks if the error code is valid and the error page path is not empty.
 *
 * @param block The block which surounds the directive
 * @param errorPage The value of the directive error_page
 */
void ConfigFileParser::readErrorPage(const Block& block, const std::string& errorPage)
{
	if (errorPage.find_first_of(s_whitespace) == std::string::npos)
		throw std::runtime_error(ERR_INVALID_ERROR_PAGE_PARAMS);

	size_t index = 0;
	while (index < errorPage.length()) {

		index = errorPage.find_first_not_of(s_whitespace, index);
		size_t errorCodeStartIndex = index;
		size_t errorCodeEndIndex = errorPage.find_first_of(s_whitespace, index);
		std::string errorCodeStr = errorPage.substr(errorCodeStartIndex, errorCodeEndIndex - errorCodeStartIndex);

		statusCode errorCode = stringToStatusCode(errorCodeStr);
		if (errorCode < StatusMovedPermanently || errorCode > StatusNonSupportedVersion)
			throw std::runtime_error(ERR_INVALID_ERROR_CODE);

		index = errorPage.find_first_not_of(s_whitespace, errorCodeEndIndex);
		if (index == std::string::npos)
			throw std::runtime_error(ERR_ERROR_PAGE_NO_PATH);

		size_t errorPagePathStartIndex = index;
		size_t errorPagePathEndIndex = errorPage.find_first_of(s_whitespace, index);
		std::string errorPagePath
			= errorPage.substr(errorPagePathStartIndex, errorPagePathEndIndex - errorPagePathStartIndex);
		if (errorPagePath.at(0) != '/')
			throw std::runtime_error(ERR_ERROR_PAGE_PATH_NO_SLASH);

		index = errorPagePathEndIndex;

		if (block == ServerBlock)
			m_configFile.servers[m_serverIndex].errorPage.insert(
				std::pair<statusCode, std::string>(errorCode, errorPagePath));
		else if (block == LocationBlock)
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].errorPage.insert(
				std::pair<statusCode, std::string>(errorCode, errorPagePath));
	}
}

/**
 * @brief Reads the return codes and corresponding return urls
 *
 * The function handles four cases:
 *
 * 1. Return code and return url
 * 2. Return code and return text
 * 2. Only return url
 * 3. Only return code
 *
 * When a return code is present, its validity is checked.
 *
 * If only a return URL is provided, it is verified to ensure it starts with either 'http://' or 'https://'.
 *
 * When a text in double quotes is provided, there is a check to ensure they are closed.
 * If that is the case, the double quotes get removed and the text is saved.
 *
 * @param returns The value of the directive returns
 */

void ConfigFileParser::readReturns(const std::string& returns)
{
	size_t index = returns.find_first_not_of(s_whitespace);
	size_t returnCodeStartIndex = index;
	size_t returnCodeEndIndex = returns.find_first_of(s_whitespace, index);

	if (returnCodeEndIndex != std::string::npos) {
		std::string returnCodeStr = returns.substr(returnCodeStartIndex, returnCodeEndIndex - returnCodeStartIndex);

		statusCode returnCode = stringToStatusCode(returnCodeStr);
		if (returnCode < StatusOK || returnCode > StatusNonSupportedVersion)
			throw std::runtime_error(ERR_INVALID_RETURN_CODE);

		size_t returnUrlOrTextStartIndex = returns.find_first_not_of(s_whitespace, returnCodeEndIndex);
		size_t returnUrlOrTextEndIndex = returns.length();
		std::string returnUrlOrText
			= returns.substr(returnUrlOrTextStartIndex, returnUrlOrTextEndIndex - returnUrlOrTextStartIndex);

		if (returnUrlOrText.at(0) != '"' && returnUrlOrText.find_first_of(s_whitespace) != std::string::npos)
			throw std::runtime_error(ERR_INVALID_RETURN_PARAMS);
		if (returnUrlOrText.find('"') != std::string::npos) {
			if (returnUrlOrText.at(returnUrlOrText.length() - 1) != '"')
				throw std::runtime_error(ERR_INVALID_RETURN_PARAMS);
			removeEnclosingDoubleQuotes(returnUrlOrText);
		}

		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.first = returnCode;
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.second = returnUrlOrText;

		return;
	}

	size_t endIndex = returns.length();
	size_t startIndex = returnCodeStartIndex;

	std::string returnCodeOrUrl = returns.substr(startIndex, endIndex - startIndex);
	if (returnCodeOrUrl.substr(0, sizeof("http://") - 1) == "http://"
		|| returnCodeOrUrl.substr(0, sizeof("https://") - 1) == "https://") {
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.first = StatusFound;
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.second = returnCodeOrUrl;
	} else {
		statusCode returnCode = stringToStatusCode(returnCodeOrUrl);
		if (returnCode < StatusOK || returnCode > StatusNonSupportedVersion)
			throw std::runtime_error(ERR_INVALID_RETURN_CODE);
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.first = returnCode;
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].returns.second = "";
	}
}

/**
 * @brief Reads the CGI extension
 *
 * The function checks if there is only one CGI extension and if it starts with a dot and does not contain any other
 * dots, otherwise it will throw an exception
 *
 * @param extension The value of the directive cgi_extension
 */
void ConfigFileParser::readCGIExtension(const std::string& extension)
{
	if (extension.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_CGI_EXTENSIONS);
	if (extension.at(0) != '.')
		throw std::runtime_error(ERR_INVALID_CGI_EXTENSION);

	std::string extensionWithoutDotAtBeginning = extension.substr(1);
	if (extensionWithoutDotAtBeginning.find_first_of('.') != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_DOTS_IN_CGI_EXTENSION);

	m_configFile.servers[m_serverIndex].locations[m_locationIndex].cgiExt = extension;
}

/**
 * @brief Reads the CGI path
 *
 * @param path The value of the directive cgi_path
 */
void ConfigFileParser::readCGIPath(const std::string& path)
{
	if (path.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error(ERR_MULTIPLE_CGI_PATHS);
	if (path.at(0) != '/')
		throw std::runtime_error(ERR_CGI_PATH_NO_SLASH);

	m_configFile.servers[m_serverIndex].locations[m_locationIndex].cgiPath = path;
}

/**
 * @brief Reads the indices
 *
 * Before reading the indices, the function clears the vector of indices
 *
 * @param indices The value of the directive index
 */
void ConfigFileParser::readIndex(const std::string& indices)
{
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].indices.clear();

	size_t index = 0;
	while (index < indices.length()) {

		index = indices.find_first_not_of(s_whitespace, index);
		size_t indicesStartIndex = index;
		size_t indicesEndIndex = indices.find_first_of(s_whitespace, index);
		std::string indexStr = indices.substr(indicesStartIndex, indicesEndIndex - indicesStartIndex);

		m_configFile.servers[m_serverIndex].locations[m_locationIndex].indices.push_back(indexStr);

		index = indicesEndIndex;
	}
}

/**
 * @brief Reads and checks the value of the server directive in the current line of the config file
 *
 * @details This function is called when the server directive is valid.
 *          It calls the appropriate function to read the value of the server directive.
 *          It throws an exception if the value is invalid.

 * @param directive Server directive which value is being read and checked
 */
void ConfigFileParser::readServerDirectiveValue(const std::string& directive, const std::string& value)
{
	if (directive == "listen")
		readListen(value);
	else if (directive == "root") {
		m_serverRootCount++;
		readRootPath(ServerBlock, value);
	} else if (directive == "server_name")
		readServerName(value);
	else if (directive == "client_max_body_size")
		readMaxBodySize(ServerBlock, value);
	else if (directive == "error_page")
		readErrorPage(ServerBlock, value);
}

/**
 * @brief Reads and checks the value of the location directive in the current line of the config file
 *
 * @details This function is called when the location directive is valid.
 *          It calls the appropriate function to read the value of the location directive.
 *          It throws an exception if the value is invalid.

 * @param directive Location directive which value is being read and checked
 */
void ConfigFileParser::readLocationDirectiveValue(const std::string& directive, const std::string& value)
{
	if (directive == "root") {
		m_locationRootCount++;
		readRootPath(LocationBlock, value);
	} else if (directive == "alias")
		readAliasPath(value);
	else if (directive == "client_max_body_size")
		readMaxBodySize(LocationBlock, value);
	else if (directive == "autoindex")
		readAutoIndex(value);
	else if (directive == "allow_methods")
		readAllowMethods(value);
	else if (directive == "error_page")
		readErrorPage(LocationBlock, value);
	else if (directive == "cgi_ext")
		readCGIExtension(value);
	else if (directive == "cgi_path")
		readCGIPath(value);
	else if (directive == "index")
		readIndex(value);
	else if (directive == "return")
		readReturns(value);
}

/**
 * @brief Reads the current line of the server config and does several checks
 *
 * How a line gets processed:
 * 1. The directive of the resulting string gets extracted and checked.
 * 2. The value of the directive gets extracted and checked.
 */
void ConfigFileParser::readServerConfigLine(void)
{
	const std::string directive = getDirective();
	if (!isDirectiveValid(directive, ServerBlock))
		throw std::runtime_error(ERR_INVALID_SERVER_DIRECTIVE);

	const std::string value = getValue();
	if ((value.empty() || value.find_last_not_of(s_whitespace) == std::string::npos))
		throw std::runtime_error(ERR_DIRECTIVE_NO_VALUE(directive));

	readServerDirectiveValue(directive, value);
}

/**
 * @brief Reads the current line of the location config and does several checks
 *
 * How a line gets processed:
 * 1. The directive of of the resulting string gets extracted and checked.
 * @todo FIXME: 2. The value of the directive gets extracted and checked.
 */
void ConfigFileParser::readLocationConfigLine(void)
{
	const std::string directive = getDirective();
	if (!isDirectiveValid(directive, LocationBlock))
		throw std::runtime_error(ERR_INVALID_LOCATION_DIRECTIVE);

	const std::string value = getValue();

	if (value.empty() || value.find_last_not_of(s_whitespace) == std::string::npos)
		throw std::runtime_error(ERR_DIRECTIVE_NO_VALUE(directive));

	readLocationDirectiveValue(directive, value);
}

/********************/
/* HELPER FUNCTIONS */
/********************/

/**
 * @brief Gets the directive from the line
 *
 * @return std::string The extracted directive
 */
std::string ConfigFileParser::getDirective() const
{
	std::string directive;

	const size_t firstWhiteSpaceIndex = m_currentLine.find_first_of(s_whitespace);
	if (firstWhiteSpaceIndex == std::string::npos)
		directive = m_currentLine;
	else
		directive = m_currentLine.substr(0, firstWhiteSpaceIndex);

	directive = webutils::trimLeadingWhitespaces(directive);
	webutils::trimTrailingWhiteSpaces(directive);

	return directive;
}

/**
 * @brief Gets the value from the line
 *
 * @return std::string  The extracted value
 */
std::string ConfigFileParser::getValue(void) const
{
	const size_t semicolonIndex = m_currentLine.find(';');
	std::string value;

	const size_t firstWhiteSpaceIndex = m_currentLine.find_first_of(s_whitespace);
	if (firstWhiteSpaceIndex == std::string::npos)
		value = m_currentLine.substr(0, semicolonIndex);
	else
		value = m_currentLine.substr(firstWhiteSpaceIndex, semicolonIndex - firstWhiteSpaceIndex);

	value = webutils::trimLeadingWhitespaces(value);
	webutils::trimTrailingWhiteSpaces(value);

	return value;
}

/**
 * @brief Converts the Block enum to a string
 *
 * @param block The block to convert
 * @return std::string The converted string
 */
std::string ConfigFileParser::convertBlockToString(Block block) const
{
	switch (block) {
	case HttpBlock:
		return "http";
	case ServerBlock:
		return "server";
	case LocationBlock:
		return "location";
	default:
		return "";
	}
}

/**
 * @brief Removes double quotes from a string
 *
 * The double quotes are removed from the beginning and the end of the string
 *
 * @param str The string to remove double quotes from
 */
void ConfigFileParser::removeEnclosingDoubleQuotes(std::string& str)
{
	size_t leadingDoubleQuotes = 0;
	size_t trailingDoubleQuotes = 0;

	while (leadingDoubleQuotes < str.length() && str[leadingDoubleQuotes] == '"') {
		leadingDoubleQuotes++;
	}

	size_t trailingIndex = str.length();
	while (trailingIndex > leadingDoubleQuotes && str[trailingIndex - 1] == '"') {
		trailingDoubleQuotes++;
		trailingIndex--;
	}

	if (leadingDoubleQuotes > 1 || trailingDoubleQuotes > 1)
		throw std::runtime_error(ERR_TOO_MANY_DOUBLE_QUOTES);

	str = str.substr(leadingDoubleQuotes, trailingIndex - leadingDoubleQuotes);
}

/**
 * @brief Skips the beginning of a block
 *
 * @param block The block to skip
 */
void ConfigFileParser::skipBlockBegin(Block block)
{
	while (std::isspace(m_configFileContent[m_configFileIndex]) != 0)
		m_configFileIndex++;

	if (block == HttpBlock)
		m_configFileIndex += convertBlockToString(HttpBlock).length();
	else if (block == ServerBlock)
		m_configFileIndex += convertBlockToString(ServerBlock).length();
	else if (block == LocationBlock)
		m_configFileIndex += convertBlockToString(LocationBlock).length();

	while (std::isspace(m_configFileContent[m_configFileIndex]) != 0)
		m_configFileIndex++;

	if (block == LocationBlock)
		skipLocationBlockPath(m_configFileIndex);

	m_configFileIndex++;
}

/**
 * @brief Skips the path of a location block
 *
 * @param index The index which gets incremented in the function to skip the path of a location block
 */
void ConfigFileParser::skipLocationBlockPath(size_t& index)
{
	while (std::isspace(m_configFileContent[index]) == 0)
		index++;
	while (std::isspace(m_configFileContent[index]) != 0)
		index++;
}

/**
 * @brief Checks if the location block is empty
 *
 * @param locationBlockContent The content of the location block
 * @return true When the location block is empty
 * @return false When the location block is not empty
 */
bool ConfigFileParser::isEmptyLocationBlock(const std::string& locationBlockContent) const
{
	const size_t openingCurlyBracketIndex = locationBlockContent.find('{');
	const size_t firstNonWhitespaceAfterCurly
		= locationBlockContent.find_first_not_of(s_whitespace, openingCurlyBracketIndex + 1);
	return (firstNonWhitespaceAfterCurly == std::string::npos);
}

/**
 * @brief Checks if the server block is empty
 *
 * @param serverBlockContent The content of the server block
 * @return true When the server block is empty
 * @return false When the server block is not empty
 */
bool ConfigFileParser::isEmptyServerBlock(const std::string& serverBlockContent) const
{
	const size_t openingCurlyBracketIndex = serverBlockContent.find('{');
	const size_t firstNonWhitespaceAfterCurly
		= serverBlockContent.find_first_not_of(s_whitespace, openingCurlyBracketIndex + 1);
	return (firstNonWhitespaceAfterCurly == std::string::npos);
}
