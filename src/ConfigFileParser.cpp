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
{
	const char* validServerDirectiveNames[]
		= { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root" };
	const int validServerDirectiveNamesSize = sizeof(validServerDirectiveNames) / sizeof(validServerDirectiveNames[0]);

	const char* validLocationDirectiveNames[] = { "root", "index", "cgi_ext", "cgi_path", "client_max_body_size",
		"autoindex", "allow_methods", "location", "return" };
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
 * 4. Does not contain invalid directives
 *
 * @param configFilePath Path to the config file
 * @return const ConfigFile& Created ConfigFile object
 */
const ConfigFile& ConfigFileParser::parseConfigFile(const std::string& configFilePath)
{
	std::ifstream fileStream(configFilePath.c_str());
	std::stringstream bufferStream;

	if (!fileStream.is_open())
		throw std::runtime_error("Failed to open config file");
	if (fileStream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Config file is empty");

	bufferStream << fileStream.rdbuf();
	m_configFileContent = bufferStream.str();
	fileStream.close();

	if (isBracketOpen())
		throw std::runtime_error("Open bracket(s) in config file");

	if (!isValidBlockBeginn(HttpBlock))
		return m_configFile;

	skipBlockBegin(HttpBlock);

	while (m_configFileContent[m_configFileIndex] != '}') {
		if (isValidBlockBeginn(ServerBlock))
			readServerBlock();
		else if (std::isspace(m_configFileContent[m_configFileIndex]) == 0) {
			throw std::runtime_error("Invalid directive");
		}
		m_configFileIndex++;
	}

	for (std::vector<ServerBlockConfig>::const_iterator serverIt = m_serverBlocksConfig.begin();
		 serverIt != m_serverBlocksConfig.end(); serverIt++) {
		processServerContent(*serverIt);
		m_serverIndex++;
	}

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
bool ConfigFileParser::isValidBlockBeginn(Block block)
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

	if (block == LocationBlock)
		skipLocationBlockPath(index);

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
		if (isKeyword(convertBlockToString(LocationBlock), m_configFileIndex) && isValidBlockBeginn(LocationBlock)) {
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
		throw std::runtime_error("Unexpected '}'");

	while (readAndTrimLine(serverBlockConfig.serverBlockContent, ';'))
		readServerConfigLine();

	for (std::vector<std::string>::const_iterator it = serverBlockConfig.locationBlocksContent.begin();
		 it != serverBlockConfig.locationBlocksContent.end(); ++it) {
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
 * To continue with the correct value of m_locationIndex the original value is stored in tmpIndex and will be used if
 * the location index is 0
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
		throw std::runtime_error("Unexpected '}'");

	while (readAndTrimLine(locationBlockContent, ';'))
		readLocationConfigLine();

	ConfigServer& server = m_configFile.servers[m_serverIndex];
	Location& location = server.locations[m_locationIndex];
	if (location.root == "html")
		location.root = server.root;
	else if (location.maxBodySize == 1)
		location.maxBodySize = server.maxBodySize;
	else if (location.errorPage.empty())
		location.errorPage = server.errorPage;

	if (m_locationIndex == 0)
		m_locationIndex = tmpIndex;
}

/**
 * @brief Reads the current line of the content, delimited by a provided char and removes leading and trailing spaces
 * If the end of the content is reached, the function returns false, otherwise it returns true
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
 *
 * Otherwise a new location will be created, added to the locations vector and the m_locationIndex will be incremented
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
	if (path == "/")
		m_locationIndex = 0;
	else {
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
 *
 * If at the end of the path is a slash, it removes it.
 *
 * This function can be used for the server block and location block
 *
 * @param block The block which surounds the directive
 * @param value The value of the directive root
 */
void ConfigFileParser::readRootPath(const Block& block, const std::string& value)
{
	size_t semicolonIndex = value.find(';');

	std::string rootPath = value.substr(0, semicolonIndex);

	if (rootPath.empty())
		throw std::runtime_error("'root' directive has no value");

	if (rootPath.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error("More than one root path");

	if (rootPath[rootPath.length() - 1] == '/')
		rootPath.erase(rootPath.end() - 1);

	if (block == ServerBlock)
		m_configFile.servers[m_serverIndex].root = rootPath;
	else if (block == LocationBlock)
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].root = rootPath;
}

/**
 * @brief Reads the server name
 *
 * The function checks if there is only one server name and reads it if that is the case
 * If an empty string is provided, the server name will be set to an empty string
 *
 * @param value The value of the directive server_name
 */

void ConfigFileParser::readServerName(const std::string& value)
{
	std::string serverName = value.substr(0, value.find(';'));

	serverName = webutils::trimLeadingWhitespaces(serverName);
	webutils::trimTrailingWhiteSpaces(serverName);

	if (serverName.empty())
		throw std::runtime_error("server_name value is empty");
	if (serverName.find_first_of(s_whitespace) != std::string::npos)
		throw std::runtime_error("More than one server name");
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
void ConfigFileParser::readSocket(const std::string& value)
{
	const size_t colonIndex = value.find(':');
	const size_t semicolonIndex = value.find(';');
	const size_t dot = value.find('.');

	if (colonIndex != std::string::npos) {
		std::string ipAddress = value.substr(0, colonIndex);
		if (!webutils::isIpAddressValid(ipAddress))
			throw std::runtime_error("Invalid ip address");

		if (ipAddress == "localhost")
			m_configFile.servers[m_serverIndex].host = "127.0.0.1";
		else
			m_configFile.servers[m_serverIndex].host = ipAddress;

		std::string port = value.substr(colonIndex + 1, semicolonIndex - colonIndex - 1);
		if (!webutils::isPortValid(port))
			throw std::runtime_error("Invalid port");
		m_configFile.servers[m_serverIndex].port = port;
	} else {
		if (dot == std::string::npos) {
			std::string hostOrPort = value.substr(0, semicolonIndex);
			if (hostOrPort == "localhost")
				m_configFile.servers[m_serverIndex].host = "127.0.0.1";
			else {
				if (!webutils::isPortValid(hostOrPort))
					throw std::runtime_error("Invalid port");
				m_configFile.servers[m_serverIndex].port = hostOrPort;
			}
		} else {
			std::string ipAddress = value.substr(0, semicolonIndex);
			if (!webutils::isIpAddressValid(ipAddress))
				throw std::runtime_error("Invalid ip address");

			m_configFile.servers[m_serverIndex].host = ipAddress;
		}
	}
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
 *
 * When the checks for number are passed, the function will check if there is a unit
 * If that is the case the function will check if the unit is valid and
 * only consists of one character which is a valid unit
 *
 * @param value The value of the directive client_max_body_size
 */
void ConfigFileParser::readMaxBodySize(const Block& block, const std::string& value)
{
	const size_t bytesPerKiloByte = 1024;
	const size_t semicolonIndex = value.find(';');
	std::string maxBodySize = value.substr(0, semicolonIndex);

	const size_t lastNumberIndex = maxBodySize.find_last_of(s_number);
	if (lastNumberIndex == std::string::npos)
		throw std::runtime_error("Invalid client_max_body_size value");

	const std::string number = maxBodySize.substr(0, lastNumberIndex + 1);
	if (number.find_first_not_of(s_number) != std::string::npos)
		throw std::runtime_error("Invalid client_max_body_size value");

	std::stringstream sstream(number);
	size_t size = 0;
	sstream >> size;

	if (lastNumberIndex != maxBodySize.size() - 1) {
		const std::string letter = maxBodySize.substr(lastNumberIndex + 1);
		if (letter.length() != 1)
			throw std::runtime_error("Invalid client_max_body_size unit");
		const char unit = letter.at(0);

		switch (unit) {
		case 'k':
		case 'K':
			size *= bytesPerKiloByte;
			break;
		case 'm':
		case 'M':
			size *= bytesPerKiloByte * bytesPerKiloByte;
			break;
		case 'g':
		case 'G':
			size *= bytesPerKiloByte * bytesPerKiloByte * bytesPerKiloByte;
			break;
		default:
			throw std::runtime_error("Invalid client_max_body_size unit");
		}
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
 * @param value The value of the directive autoindex
 */
void ConfigFileParser::readAutoIndex(const std::string& value)
{
	const size_t semicolonIndex = value.find(';');
	std::string autoindex = value.substr(0, semicolonIndex);

	if (autoindex == "on")
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].isAutoindex = true;
	else if (autoindex == "off")
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].isAutoindex = false;
	else
		throw std::runtime_error("Invalid autoindex value");
}

/**
 * @brief Reads the allow methods
 *
 * The function checks if the value of the directive is either "GET", "POST" or "DELETE".
 * If that is the case the function will set the corresponding allow method to true.
 * Otherwise it will throw an exception.
 *
 * Before setting the allow method to true the function will set all other allow methods to false
 *
 * @param value The value of the directive allow_methods
 */
void ConfigFileParser::readAllowMethods(const std::string& value)
{
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[0] = false;
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[1] = false;
	m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[2] = false;

	size_t semicolonIndex = value.find(';');
	std::string allowMethods = value.substr(0, semicolonIndex);
	if (allowMethods.empty())
		throw std::runtime_error("allow_methods value is empty");

	size_t index = 0;
	while (allowMethods[index] != '\0') {
		size_t startIndex = index;

		while (std::isspace(allowMethods[index]) == 0 && allowMethods[index] != '\0')
			index++;

		std::string method = value.substr(startIndex, index - startIndex);

		if (method == "GET")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[0] = true;
		else if (method == "POST")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[1] = true;
		else if (method == "DELETE")
			m_configFile.servers[m_serverIndex].locations[m_locationIndex].allowMethods[2] = true;
		else
			throw std::runtime_error("Invalid allow_methods value");

		while (std::isspace(allowMethods[index]) != 0 && allowMethods[index] != '\0')
			index++;
	}
}

/**
 * @brief Reads and checks the value of the server directive in the current line of the config file
 *
 * @details This function is called when the server directive is valid.
 *          It calls the appropriate function to read the value of the server directive.
 *          It throws an exception if the value is invalid.

 * @param directive Is the the server directive which value is being read and checked
 */
void ConfigFileParser::readServerDirectiveValue(const std::string& directive, const std::string& value)
{
	if (directive == "listen")
		readSocket(value);
	else if (directive == "root")
		readRootPath(ServerBlock, value);
	else if (directive == "server_name")
		readServerName(value);
	else if (directive == "client_max_body_size")
		readMaxBodySize(ServerBlock, value);
}

/**
 * @brief Reads and checks the value of the location directive in the current line of the config file
 *
 * @details This function is called when the location directive is valid.
 *          It calls the appropriate function to read the value of the location directive.
 *          It throws an exception if the value is invalid.

 * @param directive Is the the location directive which value is being read and checked
 */
void ConfigFileParser::readLocationDirectiveValue(const std::string& directive, const std::string& value)
{
	if (directive == "root")
		readRootPath(LocationBlock, value);
	else if (directive == "client_max_body_size")
		readMaxBodySize(LocationBlock, value);
	else if (directive == "autoindex")
		readAutoIndex(value);
	else if (directive == "allow_methods")
		readAllowMethods(value);
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
		throw std::runtime_error("Invalid server directive");

	const std::string value = getValue();

	if ((value.empty() || value.find_last_not_of(s_whitespace) == std::string::npos))
		throw std::runtime_error("'" + directive + "'" + " directive has no value");

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
		throw std::runtime_error("Invalid location directive");

	const std::string value = getValue();

	if (value.empty() || value.find_last_not_of(s_whitespace) == std::string::npos)
		throw std::runtime_error("'" + directive + "'" + " directive has no value");

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
		value = m_currentLine.substr(firstWhiteSpaceIndex, semicolonIndex - firstWhiteSpaceIndex + 1);

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
