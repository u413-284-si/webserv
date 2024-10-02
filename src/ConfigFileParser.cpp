#include "ConfigFileParser.hpp"

const char* const ConfigFileParser::whitespace = " \t\n\v\f\r";

/**
 * @brief PUBLIC Construct a new ConfigFileParser:: ConfigFileParser object.
 */
ConfigFileParser::ConfigFileParser(void)
	: m_configFile()
	, m_serverIndex(0)
	, m_locationIndex(0)
{
	const char* validServerDirectiveNames[]
		= { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root" };
	const int validServerDirectiveNamesSize = sizeof(validServerDirectiveNames) / sizeof(validServerDirectiveNames[0]);

	const char* validLocationDirectiveNames[] = { "root", "index", "cgi_ext", "cgi_path", "client_max_body_size",
		"autoindex", "limit_except", "location", "return" };
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
 * 4. Starts with http
 * 5. Contains minimum one server
 *
 * @param configFilePath Path to the config file
 * @return const ConfigFile& Created ConfigFile object
 */
const ConfigFile& ConfigFileParser::parseConfigFile(const std::string& configFilePath)
{
	m_stream.open(configFilePath.c_str());
	if (!m_stream.is_open())
		throw std::runtime_error("Failed to open config file");
	if (m_stream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Config file is empty");

	if (isBracketOpen(configFilePath))
		throw std::runtime_error("Open bracket(s) in config file");

	readAndTrimLine();
	if (getDirective(m_currentLine) != "http")
		throw std::runtime_error("Config file does not start with http");

	for (readAndTrimLine(); m_currentLine != "}"; readAndTrimLine()) {
		if (getDirective(m_currentLine) == "server") {
			ConfigServer server;
			m_configFile.servers.push_back(server);
			size_t bracketIndex = m_currentLine.find('{');

			if (bracketIndex != std::string::npos) {
				std::string withoutBracket = m_currentLine.substr(bracketIndex + 1, m_currentLine.length());
				size_t firstNotWhiteSpaceIndex = withoutBracket.find_first_not_of(whitespace);
				if (firstNotWhiteSpaceIndex == std::string::npos) {
					readAndTrimLine();
				} else
					m_currentLine = withoutBracket.substr(firstNotWhiteSpaceIndex, withoutBracket.length());
			} else
				readAndTrimLine();

			while (m_currentLine != "}") {
				readServerConfigLine();
				if (m_currentLine == "}")
					break;
				readAndTrimLine();
			}
			m_serverIndex++;
		} else if (getDirective(m_currentLine).empty())
			continue;
		else
			throw std::runtime_error("Invalid directive");
	}
	if (m_configFile.servers.empty())
		throw std::runtime_error("No server(s) in config file");

	return m_configFile;
}

/**
 * @brief Checks if there are no open brackets
 *
 * If a opening bracket is found, it is pushed onto the brackets stack
 * If a closing bracket is found, it is popped from the brackets stack
 *
 * At the end the stack should be empty. If not, there are open brackets
 *
 * @param configFilePath Path from the config file
 * @return true If there is minimum one open bracket
 * @return false If there no open bracket
 */
bool ConfigFileParser::isBracketOpen(const std::string& configFilePath)
{
	std::ifstream tmpStream;
	std::string tmpLine;
	std::stack<char> brackets;

	tmpStream.open(configFilePath.c_str());

	while (!(getline(tmpStream, tmpLine).fail())) {
		for (std::string::const_iterator it = tmpLine.begin(); it != tmpLine.end(); ++it) {
			if (*it == '{')
				brackets.push('{');
			else if (*it == '}' && brackets.empty())
				return true;
			else if (*it == '}')
				brackets.pop();
		}
	}
	return !brackets.empty();
}

/**
 * @brief Checks if the line contains a semicolon, except if the line contains the directive "location"
 *
 * @param line The current line to be checked
 * @return true If the line does not contain a semicolon and is not the directive "location"
 * @return false If the line contains a semicolon or the directive is "location"
 */
bool ConfigFileParser::isSemicolonMissing(const std::string& line) const
{
	return line.find(';') == std::string::npos && getDirective(line) != "location";
}

/**
 * @brief Reads the current line of the config file and removes leading and trailing spaces
 */
void ConfigFileParser::readAndTrimLine(void)
{
	getline(m_stream, m_currentLine);
	m_currentLine = webutils::trimLeadingWhitespaces(m_currentLine);
	webutils::trimTrailingWhiteSpaces(m_currentLine);
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
 */
void ConfigFileParser::readRootPath(Block block, const std::string& value)
{
	std::string rootPath = value;

	if (rootPath.find_first_of(whitespace) != std::string::npos)
		throw std::runtime_error("More than one root path");

	if (rootPath[rootPath.length() - 1] == '/')
		rootPath.erase(rootPath.end() - 1);

	if (block == ServerBlock)
		m_configFile.servers[m_serverIndex].root = rootPath;
	else if (block == LocationBlock)
		m_configFile.servers[m_serverIndex].locations[m_locationIndex].root = rootPath;
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
 * If that is the case, the function checks and reads the ip address.
 * Otherwise it checks and reads the port.
 *
 * @param value The value of the directive
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
		m_configFile.servers[m_serverIndex].host = ipAddress;

		std::string port = value.substr(colonIndex + 1, semicolonIndex - colonIndex - 1);
		if (!webutils::isPortValid(port))
			throw std::runtime_error("Invalid port");
		m_configFile.servers[m_serverIndex].port = port;
	} else {
		if (dot == std::string::npos) {
			std::string port = value.substr(0, semicolonIndex);
			if (!webutils::isPortValid(value))
				throw std::runtime_error("Invalid port");

			m_configFile.servers[m_serverIndex].host = "127.0.0.1";
			m_configFile.servers[m_serverIndex].port = port;

		} else {
			std::string ipAddress = value.substr(0, semicolonIndex);
			if (!webutils::isIpAddressValid(ipAddress))
				throw std::runtime_error("Invalid ip address");

			m_configFile.servers[m_serverIndex].host = ipAddress;
			m_configFile.servers[m_serverIndex].port = "80";
		}
	}
}

/**
 * @brief Reads and checks the value of the directive in the current line of the config file
 *
 * @details This function is called when the directive is valid.
 *          It calls the appropriate function to read the value of the directive.
 *          It throws an exception if the value is invalid.

 * @param directive Is the the directive which value is being read and checked
 */
void ConfigFileParser::readServerDirectiveValue(const std::string& directive, const std::string& value)
{
	if (directive == "listen") {
		readSocket(value);
	} else if (directive == "root")
		readRootPath(ServerBlock, value);
}

/**
 * @brief Gets the directive from the line
 *
 * @param line String containing the directive and value
 * @return std::string The extracted directive
 */
std::string ConfigFileParser::getDirective(const std::string& line) const
{
	std::string directive;

	const size_t firstWhiteSpaceIndex = line.find_first_of(whitespace);
	if (firstWhiteSpaceIndex == std::string::npos)
		directive = line;
	else
		directive = line.substr(0, firstWhiteSpaceIndex);

	directive = webutils::trimLeadingWhitespaces(directive);
	webutils::trimTrailingWhiteSpaces(directive);

	return directive;
}

/**
 * @brief Gets the value from the line
 *
 * @param line String containing the directive and value;
 * @return std::string  The extracted value
 */
std::string ConfigFileParser::getValue(const std::string& line) const
{
	const size_t semicolonIndex = line.find(';');
	std::string value;

	const size_t firstWhiteSpaceIndex = line.find_first_of(whitespace);
	if (firstWhiteSpaceIndex == std::string::npos)
		value = line.substr(0, semicolonIndex);
	else
		value = line.substr(firstWhiteSpaceIndex, semicolonIndex - firstWhiteSpaceIndex);

	value = webutils::trimLeadingWhitespaces(value);
	webutils::trimTrailingWhiteSpaces(value);

	return value;
}

/**
 * @brief Processes the line after the directive and value got extracted
 *
 * It can be the case that multiple directives are in the same line.
 * Therefore the line must still get read until all directives got processed.
 *
 * If the line does NOT contain a semicolon, the line and the current line get cleared.
 *
 * If the line contains a semicolon, the line and the current line get cleared until the semicolon.
 * Which means:
 * For the case that the line contains more than one directive, the already processed directive gets skipped.
 * For the case the line does not contain any more directives, the line gets cleared.
 *
 * @param line The current line
 */
void ConfigFileParser::processRemainingLine(std::string& line)
{
	size_t semicolonIndex = line.find(';');
	if (semicolonIndex == std::string::npos) {
		line = "";
		m_currentLine = line;
		return;
	}

	line.erase(0, semicolonIndex + 1);
	line = webutils::trimLeadingWhitespaces(line);
	webutils::trimTrailingWhiteSpaces(line);
	m_currentLine = line;
}

/**
 * @brief Reads the current line of the server config and does several checks
 *
 * How a line gets processed:
 * 1. Gets checked if it contains a semicolon.
 * 2. The directive of the resulting string gets extracted and checked.
 * 3. The value of the directive gets extracted and checked.
 *
 * If the line contains more than one directive, this process gets repeated for each directive.
 * Otherwise the line just gets cleared and the processing ends.
 */
void ConfigFileParser::readServerConfigLine(void)
{
	std::string line = m_currentLine;

	while (!line.empty() && line != "{" && line != "}") {
		if (isSemicolonMissing(line))
			throw std::runtime_error("Semicolon missing");

		const std::string directive = getDirective(line);
		if (directive == "location") {
			Location location;
			m_configFile.servers[m_serverIndex].locations.push_back(location);
			while (m_currentLine != "}") {
				readAndTrimLine();
				readLocationConfigLine();
			}
			m_locationIndex++;
			processRemainingLine(line);
			continue;
		}

		const std::string value = getValue(line);
		if ((value.empty() || value.find_last_not_of(whitespace) == std::string::npos))
			throw std::runtime_error("'" + directive + "'" + " directive has no value");

		if (!isDirectiveValid(directive, ServerBlock))
			throw std::runtime_error("Invalid server directive");

		readServerDirectiveValue(directive, value);

		processRemainingLine(line);
	}
}

/**
* @brief Reads the current line of the location config and does several checks
*
* How a line gets processed:
* 1. Gets checked if it contains a semicolon.
* 2. The directive of of the resulting string gets extracted and checked.
* @todo FIXME: 3. The value of the directive gets extracted and checked.
*
* If the line contains more than one directive, this process gets repeated for each directive.
* Otherwise the line just gets cleared and the processing ends.

*/
void ConfigFileParser::readLocationConfigLine(void)
{
	std::string line = m_currentLine;

	while (!line.empty() && line != "{" && line != "}") {
		if (isSemicolonMissing(line))
			throw std::runtime_error("Semicolon missing");

		const std::string directive = getDirective(line);
		const std::string value = getValue(line);

		if (value.empty() || value.find_last_not_of(whitespace) == std::string::npos)
			throw std::runtime_error("'" + directive + "'" + " directive has no value");

		if (!isDirectiveValid(directive, LocationBlock))
			throw std::runtime_error("Invalid location directive");

		// TODO: readLocationDirectiveValue(directive, value);

		processRemainingLine(line);
	}
}
