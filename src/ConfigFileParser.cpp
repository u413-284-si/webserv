#include "ConfigFileParser.hpp"

ConfigFileParser::ConfigFileParser(void){}

ConfigFileParser::~ConfigFileParser() { }

/**
 * @brief Parses the config file into a ConfigFile object and returns this object
 * 
 * @param configFilePath Path to the config file 
 * @return const ConfigFile& Created ConfigFile object
 */
const ConfigFile& ConfigFileParser::parseConfigFile(const std::string& configFilePath)
{
	 m_configFile.stream.open(configFilePath.c_str());
	if (!m_configFile.stream)
		throw std::runtime_error("Failed to open config file");
	else if (m_configFile.stream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Config file is empty");

    if (isBracketOpen(configFilePath))
		throw std::runtime_error("Open bracket(s) in config file");

    readAndTrimLine();
    if (m_configFile.currentLine != "http {")
        throw std::runtime_error("Config file does not start with 'http {'");

    m_configFile.index = 0;
    while (readAndTrimLine() && m_configFile.currentLine != "}") {
        if (m_configFile.currentLine == "server {") {
            while (readAndTrimLine() && m_configFile.currentLine != "}")
                readServerConfigLine();
            m_configFile.index++;
        }
    }

	
	return m_configFile;
}

 /**
  * @brief Checks if there are no open brackets
  *
  * If a opening bracket is found, it is pushed onto the m_brackets stack
  * If a closing bracket is found, it is popped from the m_brackets stack
  *
  * @param configFilePath Path from the config file
  * @return true If there is minimum one open bracket
  * @return false If there no open bracket
  */
bool ConfigFileParser::isBracketOpen(const std::string& configFilePath)
{
    std::ifstream tmpStream;
    std::string tmpLine;

    tmpStream.open(configFilePath.c_str());

    while (getline(tmpStream, tmpLine)) {
        for (std::string::const_iterator it = tmpLine.begin(); it != tmpLine.end(); ++it) {
			if (*it == '{')
				m_brackets.push('{');
			else if (*it == '}' && m_brackets.empty())
				return true;
			else if (*it == '}')
                m_brackets.pop();
        }
    }
	return !m_brackets.empty();
}

/**
 * @brief Reads the current line of the config file and removes leading and trailing spaces
 *
 * @return true if the line was read successfully
 * @return false otherwise
 */
bool ConfigFileParser::readAndTrimLine(void)
{
    if (!getline(m_configFile.stream, m_configFile.currentLine))
        return false;
    removeLeadingAndTrailingSpaces();
    return true;
}

/**
 * @brief Removes leading and trailing spaces
 * 		  from the current line of the config file
 */
void ConfigFileParser::removeLeadingAndTrailingSpaces(void)
{
    m_configFile.currentLine.erase(0, m_configFile.currentLine.find_first_not_of(' '));
    m_configFile.currentLine.erase(m_configFile.currentLine.find_last_not_of(' ') + 1);
}


/**
 * @brief Checks if the directive is valid for the given block
 * 
 * @param directive The directive to check
 * @param block The block which surounds the directive
 * @return true When the directive is valid
 * @return false When the directive is invalid
 */

bool ConfigFileParser::isDirectiveValid(const std::string& directive, int block) const
{
	const char* validServerDirectiveNames[] = { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root"};
    const int validServerDirectiveNamesSize = sizeof(validServerDirectiveNames) / sizeof(validServerDirectiveNames[0]);
	std::vector<std::string> validServerDirectives(validServerDirectiveNames, validServerDirectiveNames + validServerDirectiveNamesSize); 

	const char* validLocationDirectiveNames[] = { "root", "index", "cgi_ext", "cgi_path", "autoindex", "limit_except", "location", "return" };
    const int validLocationDirectiveNamesSize = sizeof(validLocationDirectiveNames) / sizeof(validLocationDirectiveNames[0]);
    std::vector<std::string> validLocationDirectives(validLocationDirectiveNames, validLocationDirectiveNames + validLocationDirectiveNamesSize);

	if (block == SERVER)
	{
		if (std::find(validServerDirectives.begin(), validServerDirectives.end(), directive) == validServerDirectives.end() && !directive.empty())
			return false;
	}
	else if (block == LOCATION)
	{
		if (std::find(validLocationDirectives.begin(), validLocationDirectives.end(), directive) == validLocationDirectives.end() && !directive.empty())
			return false;
	}
	return true;
}


/**
 * @brief Checks if the current line of the config file contains a semicolon
 * 
 * @return true If the line contains a semicolon a the end
 * @return false If the line does not contain a semicolon
 */
bool ConfigFileParser::isSemicolonAtEnd(void) const
{
	return m_configFile.currentLine.find_last_of(';') != std::string::npos || m_configFile.currentLine.empty();
}

/**
 * @brief Checks if the current line of the config file contains only one semicolon
 * 
 * @return true If the line contains only one semicolon or the line is empty
 * @return false If the line contains more than one semicolon 
 */
bool ConfigFileParser::isSemicolonCountOne(void) const
{
	ssize_t semicolonCount = std::count(m_configFile.currentLine.begin(), m_configFile.currentLine.end(), ';');

	return semicolonCount == 1 || m_configFile.currentLine.empty();
}

bool ConfigFileParser::isListenIpValid(size_t directiveLen)
{
	size_t colonIndex = m_configFile.currentLine.find(':');
	std::string ip = m_configFile.currentLine.substr(directiveLen + 1, colonIndex - directiveLen - 1);

	if (ip.find_first_not_of("0123456789.") != std::string::npos)
		return false;

	size_t firstDotIndex = ip.find('.');
	if (firstDotIndex == std::string::npos)
		return false;
	size_t secondDotIndex = ip.find('.', firstDotIndex + 1);
	if (secondDotIndex == std::string::npos)
		return false;
	size_t thirdDotIndex = ip.find('.', secondDotIndex + 1);
	if (thirdDotIndex == std::string::npos)
		return false;

	std::string firstOctetStr = m_configFile.currentLine.substr(ip[0], firstDotIndex);
	std::string secondOctetStr = m_configFile.currentLine.substr( firstDotIndex + 1, secondDotIndex - firstDotIndex - 1);
	std::string thirdOctetStr = m_configFile.currentLine.substr(secondDotIndex + 1, thirdDotIndex - secondDotIndex - 1);
	std::string fourthOctetStr = m_configFile.currentLine.substr(thirdDotIndex + 1, ip.length() - thirdDotIndex - 1);

	const int base = 10;
	long firstOctet = std::strtol(firstOctetStr.c_str(), NULL, base);
	long secondOcetet = std::strtol(secondOctetStr.c_str(), NULL, base);
	long thirdOctet = std::strtol(thirdOctetStr.c_str(), NULL, base);
	long fourthOctet = std::strtol(fourthOctetStr.c_str(), NULL, base);

	const short maxIpValue = 255;
	const short minIpValue = 0;
	if (firstOctet > maxIpValue || secondOcetet > maxIpValue || thirdOctet > maxIpValue || fourthOctet > maxIpValue)
		return false;
	if (firstOctet < minIpValue || secondOcetet < minIpValue || thirdOctet < minIpValue || fourthOctet < minIpValue)
		return false;
	
	return true;
}

/**
 * @brief Checks if the value of the listen directive is valid
 *
 * @details The value of the listen directive must not contain a character other than '0'-'9'.
 *          The value of the listen directive must be between 1-65535.
 *
 * @param directive 
 * @return true 
 * @return false 
 */
bool ConfigFileParser::isListenValueValid(const std::string& directive)
{
	size_t directiveLen = directive.length();
	size_t semicolonIndex = m_configFile.currentLine.find(';');
	std::string valueStr = m_configFile.currentLine.substr(directiveLen + 1, semicolonIndex - directiveLen - 1);

	if (valueStr.find_first_not_of("0123456789") != std::string::npos)
		return false;

	const int base = 10;
	const int maxPort = 65535;
	const int minPort = 1;

	int long value = std::strtol(valueStr.c_str(), NULL, base);
	if (value <= minPort || value > maxPort) 
		return false;

	m_configFile.servers[m_configFile.index].port = value;
	
	return true;
}

/**
 * @brief Reads and checks the value of the directive in the current line of the config file
 * 
 * @details This function is called when the directive is valid.
 *          It calls the appropriate function to read the value of the directive.
 *          It throws an exception if the value is invalid. 

 * @param directive Is the the directive which value is being read and checked
 */
void ConfigFileParser::readDirectiveValue(const std::string& directive)
{
	if (directive == "listen")
	{
		if (!isListenValueValid(directive))
			throw std::runtime_error("Invalid listen value");
	}
}

void ConfigFileParser::readServerConfigLine(void)
{
    ConfigServer server;
    std::string directive;

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
	if (!isDirectiveValid(directive, SERVER))
		throw std::runtime_error("Invalid server directive");

	if (directive == "location") {
        while (readAndTrimLine() && m_configFile.currentLine != "}")
            readLocationConfigLine();
		return;
    }

	if (!isSemicolonAtEnd())
		throw std::runtime_error("Semicolon missing");
	if (!isSemicolonCountOne())
		throw std::runtime_error("Too many semicolons");

    m_configFile.servers.push_back(server);

	readDirectiveValue(directive);
}

void ConfigFileParser::readLocationConfigLine(void)
{
    Location location;
    std::string directive;

	
    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (!isDirectiveValid(directive, LOCATION))
		throw std::runtime_error("Invalid location directive");

	if (!isSemicolonAtEnd())
		throw std::runtime_error("Semicolon missing");
	if (!isSemicolonCountOne())
		throw std::runtime_error("Too many semicolons");

    m_configFile.servers[m_configFile.index].locations.push_back(location);
}
