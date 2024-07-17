#include "ConfigFileParser.hpp"

ConfigFileParser::ConfigFileParser(void){}

ConfigFileParser::~ConfigFileParser() { }

/**
 * @brief Parses the config file into a ConfigFile object and returns this object
 * 
 * The function checks also if the config file:
 * 1. Can be opened
 * 2. Is not empty
 * 3. Does not contain open brackets
 * 4. Starts with 'http {'
 * 5. Contains minimum one server
 *
 * @param configFilePath Path to the config file 
 * @return const ConfigFile& Created ConfigFile object
 */
const ConfigFile& ConfigFileParser::parseConfigFile(const std::string& configFilePath)
{
	m_configFile.stream.open(configFilePath.c_str());
	if (!m_configFile.stream.is_open())
		throw std::runtime_error("Failed to open config file");
	if (m_configFile.stream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Config file is empty");

    if (isBracketOpen(configFilePath))
		throw std::runtime_error("Open bracket(s) in config file");

    readAndTrimLine();
    if (m_configFile.currentLine != "http {")
        throw std::runtime_error("Config file does not start with 'http {'");

    m_configFile.serverIndex = 0;
	for (readAndTrimLine(); m_configFile.currentLine != "}"; readAndTrimLine())
	{
		if (m_configFile.currentLine == "server {")
		{
		 for (readAndTrimLine(); m_configFile.currentLine != "}"; readAndTrimLine())
				readServerConfigLine();
			m_configFile.serverIndex++;
		}
	}
	if (m_configFile.servers.empty())
		throw std::runtime_error("No server(s) in config file");
	
	return m_configFile;
}

/**
 * @brief Initializes the ConfigServer object with default values
 * 
 * @param configServer The ConfigServer object to initialize 
 */
void ConfigFileParser::initializeConfigServer(ConfigServer &configServer)
{
	const char* validServerDirectiveNames[] = { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root"};
    const int validServerDirectiveNamesSize = sizeof(validServerDirectiveNames) / sizeof(validServerDirectiveNames[0]);

	configServer.validServerDirectives = std::vector<std::string>(validServerDirectiveNames, validServerDirectiveNames + validServerDirectiveNamesSize); 
	configServer.serverName = "";
	configServer.root = "html";
	configServer.listen.insert(std::make_pair("127.0.0.1", 80));
	configServer.locationIndex = 0;
	configServer.maxBodySize = 1;
	configServer.errorPage = std::map<unsigned short, std::string>();
	configServer.locationIndex = 0;
	configServer.locations = std::vector<Location>();

    m_configFile.servers.push_back(configServer);
}

/**
 * @brief Initializes the Location object with default values
 * 
 * @param location The Location object to initialize
 */
void ConfigFileParser::initializeLocation(Location &location)
{
	const char* validLocationDirectiveNames[] = { "root", "index", "cgi_ext", "cgi_path", "autoindex", "limit_except", "location", "return" };
    const int validLocationDirectiveNamesSize = sizeof(validLocationDirectiveNames) / sizeof(validLocationDirectiveNames[0]);

	location.path = "";
	location.root = "html";
	location.indices = std::vector<std::string>();
	location.cgiExt = "";
	location.cgiPath = "";
	location.isAutoindex = false;
	location.returns = std::map<unsigned short, std::string>();
    location.validLocationDirectives = std::vector<std::string>(validLocationDirectiveNames, validLocationDirectiveNames + validLocationDirectiveNamesSize);
	m_configFile.servers[m_configFile.serverIndex].locations.push_back(location);
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
 * @details The current line of the config file is read.
 * 
 *          If the line contains a semicolon:
 *          1. Get substring from the current line to the semicolon
 *          2. Semicolon gets removed from the string because it used as a delimiter for getline
 *          4. Leading and trailing spaces get removed
 *          5. Semicolon is added to the end of the string. 
 *          This allows to handle directives on the same line separated by semicolons.
 *
 *          If the line does NOT contain a semicolon, it is normally read and trimmed.
 */
void ConfigFileParser::readAndTrimLine(void)
{
	getline(m_configFile.stream, m_configFile.currentLine);
	if (!m_configFile.currentLine.empty() && isSemicolonAtEnd())
	{
		size_t semicolonIndex = m_configFile.currentLine.find(';');

		m_configFile.currentLine = m_configFile.currentLine.substr(0, semicolonIndex);
		removeLeadingAndTrailingSpaces();
		m_configFile.currentLine += ';';
	}
	else {
		removeLeadingAndTrailingSpaces();
	}
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
 * Does not check directive if it only contains whitespaces.
 * This is for the case when there is an line in the config file which contains only whitespaces.
 *
 * @param directive The directive to check
 * @param block The block which surounds the directive
 * @return true When the directive is valid
 * @return false When the directive is invalid
 */

bool ConfigFileParser::isDirectiveValid(const std::string& directive, int block) const
{
	if (block == SERVER)
	{
		std::vector<std::string> validServerDirectives = m_configFile.servers[m_configFile.serverIndex].validServerDirectives;
		if (std::find(validServerDirectives.begin(), validServerDirectives.end(), directive) == validServerDirectives.end()
			&& directive.find_first_not_of(" \t\n\v\f\r") != std::string::npos)
			return false;
	}
	else if (block == LOCATION)
	{
		std::vector<std::string> validLocationDirectives = m_configFile.servers[m_configFile.serverIndex].locations[m_configFile.servers[m_configFile.serverIndex].locationIndex].validLocationDirectives;
		if (std::find(validLocationDirectives.begin(), validLocationDirectives.end(), directive) == validLocationDirectives.end()
			&& directive.find_first_not_of(" \t\n\v\f\r") != std::string::npos)
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
	return m_configFile.currentLine.find_last_of(';') != std::string::npos;
}

/**
 * @brief Checks if the provided ip address of the listen directive is valid and reads it if that is the case
 * 
 * Because the listen directive can contain only an ip address, only a port or can contain both, it must be validated if a colon is present.
 * If no colon is present, the port or the ip address must be valid.
 *
 * The function makes sure that the ip address is valid in the following ways:
 * 1. The ip address must not contain any character other than '0'-'9' or '.'
 * 2. The number of dots must be 3
 * 3. The numbers within the ip address can not be smaller than 0 or greater than 255
 *
 * @param ip The ip address to check
 * @return true If the ip address is valid
 * @return false If the ip address is not valid
 */
bool ConfigFileParser::isIpAddressValid(const std::string& ip) const
{
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

	std::string firstOctetStr = ip.substr(0, firstDotIndex);
	std::string secondOctetStr = ip.substr( firstDotIndex + 1, secondDotIndex - firstDotIndex - 1);
	std::string thirdOctetStr = ip.substr(secondDotIndex + 1, thirdDotIndex - secondDotIndex - 1);
	std::string fourthOctetStr = ip.substr(thirdDotIndex + 1, ip.length() - thirdDotIndex - 1);

	const int base = 10;
	long firstOctet = std::strtol(firstOctetStr.c_str(), NULL, base);
	long secondOctet = std::strtol(secondOctetStr.c_str(), NULL, base);
	long thirdOctet = std::strtol(thirdOctetStr.c_str(), NULL, base);
	long fourthOctet = std::strtol(fourthOctetStr.c_str(), NULL, base);

	const short maxIpValue = 255;
	const short minIpValue = 0;
	if (firstOctet > maxIpValue || secondOctet > maxIpValue || thirdOctet > maxIpValue || fourthOctet > maxIpValue)
		return false;
	if (firstOctet < minIpValue || secondOctet < minIpValue || thirdOctet < minIpValue || fourthOctet < minIpValue)
		return false;

	return true;
}

/**
 * @brief Checks if the ip address is valid and reads it if that is the case
 *
 * For the case that no semicolon is can be found in the listen directive, the function checks
 * if the defined values is valid port. If that is true, the function returns.
 * If that is not true, the function checks if it is a valid ip address and reads it.
 * 
 */
void ConfigFileParser::readIpAddress(void)
{
	size_t colonIndex = m_configFile.currentLine.find(':'); 
	size_t numIndex = m_configFile.currentLine.find_first_of("0123456789");
	std::string ip;

	if (colonIndex == std::string::npos)
	{
		size_t semicolonIndex = m_configFile.currentLine.find(';');
		std::string value = m_configFile.currentLine.substr(numIndex, semicolonIndex - numIndex);
		if (isPortValid(value))
			return ;
		if (m_configFile.currentLine.find('.') == std::string::npos)
			throw std::runtime_error("Invalid port");
		ip = value;
	}
	else
		ip = m_configFile.currentLine.substr(numIndex, colonIndex - numIndex);

	if (!isIpAddressValid(ip))
		throw std::runtime_error("Invalid ip address");

	m_configFile.servers[m_configFile.serverIndex].listen.insert(std::make_pair(ip, 0));
}

/**
 * @brief Checks if the value of the listen directive is valid and reads it if that is the case
 *
 * The function makes sure that the port is valid in the following ways:
 * 1. The port must not contain a character other than '0'-'9'
 * 2. The value of the port must be between 1-65535
 *
 * @param port The port to be checked
 * @return true If the port is valid
 * @return false If the port is invalid
 */
bool ConfigFileParser::isPortValid(const std::string& port) const
{
	if (port.find_first_not_of("0123456789") != std::string::npos)
		return false;

	const int base = 10;
	const int maxPort = 65535;
	const int minPort = 1;

	int long portNum = std::strtol(port.c_str(), NULL, base);
	return !(portNum <= minPort || portNum > maxPort);
}

/**
 * @brief Checks if the port is valid and reads it if that is the case
 *
 * For the case that no semicolon is can be found in the listen directive, the function checks
 * if the defined values is valid ip address. If that is true, the function returns.
 * If that is not true, the function and checks if it is a valid port and reads it.
 * 
 */
void ConfigFileParser::readPort(void)
{
	size_t colonIndex = m_configFile.currentLine.find(':');
	size_t numIndex = m_configFile.currentLine.find_first_of("0123456789");
	size_t semicolonIndex = m_configFile.currentLine.find(';');
	std::string port;


	if (colonIndex == std::string::npos)
	{
		std::string value = m_configFile.currentLine.substr(numIndex, semicolonIndex - numIndex);
		if (isIpAddressValid(value))
			return;
		if (m_configFile.currentLine.find('.') != std::string::npos)
			throw std::runtime_error("Invalid ip address");
		port = value;
	}
	else
		port = m_configFile.currentLine.substr(colonIndex + 1, semicolonIndex - colonIndex - 1);

	if (!isPortValid(port))
		throw std::runtime_error("Invalid port");

	for (std::map<std::string, unsigned short>::iterator it = m_configFile.servers[m_configFile.serverIndex].listen.begin(); it != m_configFile.servers[m_configFile.serverIndex].listen.end(); it++)
		if (it->second == 0)
			it->second = std::atoi(port.c_str());
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
		readIpAddress();
		readPort();
	}
}

/**
 * @brief Reads the current line of the server config and does several checks
 *
 * The function checks following things:
 * 1. That the directive is valid
 * 2. That at the end of the directive there is a semicolon 
 * 3. That the value of the directive is valid
 *
 * It also checks if the server block contains the location directive.
 * If this is the case, it calls the function readLocationConfigLine() whithin a loop to read and check the location block.
 */
void ConfigFileParser::readServerConfigLine(void)
{
    ConfigServer server;
    std::string directive;

	initializeConfigServer(server);

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
	if (!isDirectiveValid(directive, SERVER))
		throw std::runtime_error("Invalid server directive");

	if (directive == "location") {
		for (readAndTrimLine(); m_configFile.currentLine != "}"; readAndTrimLine())
			readLocationConfigLine();
		m_configFile.servers[m_configFile.serverIndex].locationIndex++;
		return;
    }

	if (!m_configFile.currentLine.empty() && !isSemicolonAtEnd())
		throw std::runtime_error("Semicolon missing");

	readDirectiveValue(directive);
}

/**
 * @brief Reads the current line of the location and does several checks
 *
 * The function checks following things:
 * 1. That the directive is valid
 * 2. That at the end of the directive there is a semicolon 
 * 3. That the value of the directive is valid
 *
 * It also checks if the server block contains the location directive.
 * If this is the case, it calls the function readLocationConfigLine() whithin a loop to read and check the location block.
 */
void ConfigFileParser::readLocationConfigLine(void)
{
    Location location;
    std::string directive;

	initializeLocation(location);
	
    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (!isDirectiveValid(directive, LOCATION))
		throw std::runtime_error("Invalid location directive");

	if (!m_configFile.currentLine.empty() && !isSemicolonAtEnd())
		throw std::runtime_error("Semicolon missing");

    m_configFile.servers[m_configFile.serverIndex].locations.push_back(location);
}
