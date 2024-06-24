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

size_t ConfigFileParser::countChars(const std::string& line, char c)
{
	size_t count = 0;
	for (std::string::const_iterator it = line.begin(); it != line.end(); ++it) {
		if (*it == c)
			count++;
	}
	return count;
}

void ConfigFileParser::readServerConfig(size_t index)
{
    ConfigServer server;
    std::string directive;
    const char* validServerDirectives[] = { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root", "location" };
    const int validServerDirectivesSize = sizeof(validServerDirectives) / sizeof(validServerDirectives[0]);

    std::set<std::string> validServerDirectivesSet(validServerDirectives, validServerDirectives + validServerDirectivesSize);

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
	if (std::find(validServerDirectivesSet.begin(), validServerDirectivesSet.end(), directive) == validServerDirectivesSet.end() && !directive.empty())
		throw std::runtime_error("Invalid server directive");
	
	if (m_configFile.currentLine.find_last_of(';') == std::string::npos)
        throw std::runtime_error("Missing semicolon(s)");
	else if (countChars(m_configFile.currentLine, ';') > 1)
        throw std::runtime_error("Too many semicolons in line");

    m_configFile.servers.push_back(server);

    if (directive == "location") {
        while (readAndTrimLine() && m_configFile.currentLine != "}")
            readLocationConfig(index);
    }
}

void ConfigFileParser::readLocationConfig(size_t index)
{
    Location location;
    std::string directive;
    const char* validLocationDirectives[] = { "root", "index", "cgi_ext", "cgi_path", "autoindex", "limit_except", "location", "return" };
    const int validLocationDirectivesSize = sizeof(validLocationDirectives) / sizeof(validLocationDirectives[0]);

    std::set<std::string> validLocationDirectivesSet(validLocationDirectives, validLocationDirectives + validLocationDirectivesSize);

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (std::find(validLocationDirectivesSet.begin(), validLocationDirectivesSet.end(), directive) == validLocationDirectivesSet.end())
        throw std::runtime_error("Invalid location directive");

    m_configFile.servers[index].locations.push_back(location);
}
