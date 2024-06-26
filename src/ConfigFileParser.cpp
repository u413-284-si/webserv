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

    size_t index = 0;
    while (readAndTrimLine() && m_configFile.currentLine != "}") {
        if (m_configFile.currentLine == "server {") {
            while (readAndTrimLine() && m_configFile.currentLine != "}")
                readServerConfig(index);
            index++;
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
 * @brief Counts the number of times a character appears in a string 
 * 
 * @param line The string which contains the character to count
 * @param character The character to count within the string
 * @return size_t The number of times the character appears in the string
 */
size_t ConfigFileParser::countChars(const std::string& line, char character)
{
	size_t count = 0;
	for (std::string::const_iterator it = line.begin(); it != line.end(); ++it) {
		if (*it == character)
			count++;
	}
	return count;
}

/**
 * @brief Checks if the directive is valid for the given block
 * 
 * @param directive The directive to check
 * @param block The block which surounds the directive
 * @return true When the directive is valid
 * @return false When the directive is invalid
 */

bool ConfigFileParser::isDirectiveValid(const std::string& directive, int block)
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

void ConfigFileParser::readServerConfig(size_t index)
{
    ConfigServer server;
    std::string directive;

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
	if (!isDirectiveValid(directive, SERVER))
		throw std::runtime_error("Invalid server directive");

	if (directive == "location") {
        while (readAndTrimLine() && m_configFile.currentLine != "}")
            readLocationConfig(index);
		return;
    }
	if (m_configFile.currentLine.find_last_of(';') == std::string::npos && !m_configFile.currentLine.empty())
        throw std::runtime_error("Missing semicolon(s)");
	else if (std::count(m_configFile.currentLine.begin(), m_configFile.currentLine.end(), ';') > 1)
        throw std::runtime_error("Too many semicolons in line");

    m_configFile.servers.push_back(server);
}

void ConfigFileParser::readLocationConfig(size_t index)
{
    Location location;
    std::string directive;

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (!isDirectiveValid(directive, LOCATION))
        throw std::runtime_error("Invalid location directive");

    m_configFile.servers[index].locations.push_back(location);
}
