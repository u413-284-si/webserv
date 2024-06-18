#include "ConfigFileParser.hpp"

/**
 * @brief Reads and parses the config file to the m_configFile struct
 * 
 * @param configFilePath path to the config file
 */
ConfigFileParser::ConfigFileParser(const std::string& configFilePath)
{
    m_configFile.stream.open(configFilePath.c_str());
    if (!m_configFile.stream)
		throw std::runtime_error("Error: Failed to open config file");
	else if (m_configFile.stream.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Error: Config file is empty");
	
	checkBrackets(configFilePath);

	readAndTrimLine();
	if (m_configFile.currentLine != "http {")
		throw std::runtime_error("Error: Config file does not start with 'http {'");

	size_t index = 0;
    while (readAndTrimLine() && m_configFile.currentLine != "}") {
        if (m_configFile.currentLine == "server {") {
			while (readAndTrimLine() && m_configFile.currentLine != "}")
				readServerConfig(index);
			index++;
        }
    }

    if (m_brackets.size() != 0)
        throw std::runtime_error("Error: Missing bracket(s) in config file");
}

ConfigFileParser::~ConfigFileParser() { }

/**
 * @brief Checks if there are no open brackets 
 * 
 * If a opening bracket is found, it is pushed onto the m_brackets stack
 * If a closing bracket is found, it is popped from the m_brackets stack
 * 
 * @param configFilePath path to the config file
 */
void ConfigFileParser::checkBrackets(const std::string& configFilePath)
{
	std::ifstream tmpStream;
	std::string tmpLine;

	tmpStream.open(configFilePath.c_str());

    while (getline(tmpStream, tmpLine))
	{
		for (std::string::const_iterator it = tmpLine.begin(); it != tmpLine.end(); ++it)
		{
			if (*it == '{')
				m_brackets.push('{');
			else if (*it == '}')
				m_brackets.pop();
		}
	}
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

void ConfigFileParser::readServerConfig(size_t index)
{
    Server server;
    std::string directive;
    const char* validServerDirectives[] = { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root", "location" };
    const int validServerDirectivesSize = sizeof(validServerDirectives) / sizeof(validServerDirectives[0]);

    std::set<std::string> validServerDirectivesSet(validServerDirectives, validServerDirectives + validServerDirectivesSize);

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (std::find(validServerDirectivesSet.begin(), validServerDirectivesSet.end(), directive) == validServerDirectivesSet.end() && !directive.empty())	
        throw std::runtime_error("Error: Invalid server directive");

    m_configFile.servers.push_back(server);

	if (directive == "location")
	{
		while (readAndTrimLine() && m_configFile.currentLine != "}")
			readLocationConfig(index);
	}
}

void ConfigFileParser::readLocationConfig(size_t index)
{
    Location location;
    std::string directive;
    const char* validLocationDirectives[] = { "root", "index", "cgi_ext", "cgi_path", "autoindex", "limit_except", "location", "return"};
    const int validLocationDirectivesSize = sizeof(validLocationDirectives) / sizeof(validLocationDirectives[0]);

    std::set<std::string> validLocationDirectivesSet(validLocationDirectives, validLocationDirectives + validLocationDirectivesSize);

    directive = m_configFile.currentLine.substr(0, m_configFile.currentLine.find(' '));
    if (std::find(validLocationDirectivesSet.begin(), validLocationDirectivesSet.end(), directive) == validLocationDirectivesSet.end())
        throw std::runtime_error("Error: Invalid location directive");

    m_configFile.servers[index].locations.push_back(location);
}