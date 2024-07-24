#include "ConfigFileParser.hpp"

/**
 * @brief PUBLIC Construct a new ConfigFileParser:: ConfigFileParser object.
 */
ConfigFileParser::ConfigFileParser(void){}

/**
 * @brief PRIVATE Construct a new ConfigFileParser:: ConfigFileParser object with copy.
 * 
 * The copy constructor is private to prevent direct object creation.
 * @param ref The ConfigFileParser object to copy
 */
ConfigFileParser::ConfigFileParser(const ConfigFileParser &ref)
{
	static_cast<void>(ref);
}

/**
 * @brief PRIVATE Overload of the assignment operator.
 * 
 * The assigment operator is private to prevent direct object creation.
 * @param ref The ConfigFileParser object to assign.
 * @return ConfigFileParser&  The ConfigFileParser object.
 */
ConfigFileParser& ConfigFileParser::operator=(const ConfigFileParser &ref)
{
	static_cast<void>(ref);
	return *this;
}

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
bool ConfigFileParser::isBracketOpen(const std::string& configFilePath) const
{
    std::ifstream tmpStream;
    std::string tmpLine;
	std::stack<char> brackets;

    tmpStream.open(configFilePath.c_str());

    while (getline(tmpStream, tmpLine)) {
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
 * @brief Checks if the line contains a semicolon
 * 
 * @param line The current line to be checked
 * @return true If the line does not contain a semicolon
 * @return false If the line contains a semicolon
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
	getline(m_configFile.stream, m_configFile.currentLine);
	removeLeadingAndTrailingSpaces(m_configFile.currentLine);
}

/**
 * @brief Removes leading and trailing spaces
 * 		  from the current line of the config file
 */
void ConfigFileParser::removeLeadingAndTrailingSpaces(std::string &line) const
{
    line.erase(0, line.find_first_not_of(' '));
    line.erase(line.find_last_not_of(' ') + 1);
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
void ConfigFileParser::readIpAddress(const std::string &value)
{
	size_t colonIndex = value.find(':'); 
	size_t numIndex = value.find_first_of("0123456789");
	std::string ip;

	if (colonIndex == std::string::npos)
	{
		size_t semicolonIndex = value.find(';');
		std::string num = value.substr(numIndex, semicolonIndex - numIndex);
		if (isPortValid(num))
			return ;
		if (value.find('.') == std::string::npos)
			throw std::runtime_error("Invalid port");
		ip = num;
	}
	else
		ip = value.substr(numIndex, colonIndex - numIndex);

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
void ConfigFileParser::readPort(const std::string &value)
{
	size_t colonIndex = value.find(':');
	size_t numIndex = value.find_first_of("0123456789");
	size_t semicolonIndex = value.find(';');
	std::string port;


	if (colonIndex == std::string::npos)
	{
		std::string num = value.substr(numIndex, semicolonIndex - numIndex);
		if (isIpAddressValid(num))
			return;
		if (value.find('.') != std::string::npos)
			throw std::runtime_error("Invalid ip address");
		port = num;
	}
	else
		port = value.substr(colonIndex + 1, semicolonIndex - colonIndex - 1);

	if (!isPortValid(port))
		throw std::runtime_error("Invalid port");

	for (std::map<std::string, unsigned short>::iterator it = m_configFile.servers[m_configFile.serverIndex].listen.begin(); it != m_configFile.servers[m_configFile.serverIndex].listen.end(); it++)
		if (it->second == 0)
			it->second = std::atoi(port.c_str());
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
void ConfigFileParser::readRootPath(int block, const std::string &value)
{
	std::string rootPath = value;

	if (rootPath.find_first_of("  \t\n\v\f\r") != std::string::npos)
		throw std::runtime_error("More than one root path");

	if (rootPath[rootPath.length() - 1] == '/')
		rootPath = rootPath.substr(0, rootPath.length() - 1);

	if (block == SERVER)
		m_configFile.servers[m_configFile.serverIndex].root = rootPath;
	else if (block == LOCATION)
		m_configFile.servers[m_configFile.serverIndex].locations[m_configFile.servers[m_configFile.serverIndex].locationIndex].root = rootPath;
}

/**
 * @brief Reads and checks the value of the directive in the current line of the config file
 * 
 * @details This function is called when the directive is valid.
 *          It calls the appropriate function to read the value of the directive.
 *          It throws an exception if the value is invalid. 

 * @param directive Is the the directive which value is being read and checked
 */
void ConfigFileParser::readServerDirectiveValue(const std::string& directive, const std::string &value)
{
	if (directive == "listen")
	{
		readIpAddress(value);
		readPort(value);
	}
	else if (directive == "root")
		readRootPath(SERVER, value);
}



/**
 * @brief Gets the directive from the line
 * 
 * @param line String containing the directive and value
 * @return std::string The extracted directive
 */
std::string ConfigFileParser::getDirective(const std::string& line) const
{
	size_t firstWhiteSpaceIndex = line.find_first_of(" \t\n\v\f\r");
	std::string directive = line.substr(0, firstWhiteSpaceIndex);

	removeLeadingAndTrailingSpaces(directive);

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
	if (getDirective(line) == "location")
		return line;

	size_t semicolonIndex = line.find(';');
	
	size_t firstWhiteSpaceIndex = line.find_first_of(" \t\n\v\f\r");
	std::string value = line.substr(firstWhiteSpaceIndex, semicolonIndex - firstWhiteSpaceIndex);

	removeLeadingAndTrailingSpaces(value);

	return value;
}

/**
 * @brief Processes the line after the directive and value got extracted
 *
 * It can be the case that multiple directorives are in the same line.
 * Therefore the line must still get read until all directives got processed.
 * 
 * If the line still contains more then one directive, the processed directive gets skipped.
 * If the line does not contain any more directives, the line gets cleared.
 * 
 * @param line The current line
 */
void ConfigFileParser::processRemainingLine(std::string& line) const
{
	size_t semicolonIndex = line.find(';');

	if (line.size() > semicolonIndex + 1)
	{
		line = line.substr(semicolonIndex + 1);
		removeLeadingAndTrailingSpaces(line);
	}
	else
		line.clear();
}


/**
 * @brief Reads the current line of the server config and does several checks
 *
 * How a line gets processed:
 * 1. Gets checked if it contains a semicolon.
 * 2. The directive of of the resulting string gets extracted and checked.
 * 3. The value of the directive gets extracted and checked.
 * 
 * If the line contains more then one directive, this process gets repeated for each directive.
 * Otherwise the line just gets cleared and the processing ends.
 */
void ConfigFileParser::readServerConfigLine(void)
{
    ConfigServer server;
    std::string directive;
	std::string value;
	std::string line = m_configFile.currentLine;

	initializeConfigServer(server);

	while (!line.empty())
	{
		if (isSemicolonMissing(line))
			throw std::runtime_error("Semicolon missing");

		directive = getDirective(line);
		value = getValue(line);

		if ((value.empty() || value.find_last_not_of(" \t\n\v\f\r") == std::string::npos ) && directive != "location")
			throw std::runtime_error("'" + directive + "'" + " directive has no value");

		if (!isDirectiveValid(directive, SERVER))
			throw std::runtime_error("Invalid server directive");
		if (directive == "location") {
			for (readAndTrimLine(); m_configFile.currentLine != "}"; readAndTrimLine())
				readLocationConfigLine();
			m_configFile.servers[m_configFile.serverIndex].locationIndex++;
			break;
   		}

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
 * 3. The value of the directive gets extracted and checked.
 * 
 * If the line contains more then one directive, this process gets repeated for each directive.
 * Otherwise the line just gets cleared and the processing ends.

 */
void ConfigFileParser::readLocationConfigLine(void)
{
    Location location;
    std::string directive;
	std::string value;
	std::string line = m_configFile.currentLine;

	initializeLocation(location);
	
	while (!line.empty())
	{
		if (isSemicolonMissing(line))
			throw std::runtime_error("Semicolon missing");

		directive = getDirective(line);
		value = getValue(line);

		if (value.empty() || value.find_last_not_of(" \t\n\v\f\r") == std::string::npos)
			throw std::runtime_error("'" + directive + "'" + " directive has no value");

		if (!isDirectiveValid(directive, LOCATION))
			throw std::runtime_error("Invalid location directive");
				
		readServerDirectiveValue(directive, value);

		processRemainingLine(line);
	}

    m_configFile.servers[m_configFile.serverIndex].locations.push_back(location);
}
