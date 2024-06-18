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
 * @brief Removes leading and trailing spaces from a string
 * 
 * @param str string with potential leading and trailing spaces
 * @return std::string string without leading and trailing spaces
 */
std::string ConfigFileParser::removeLeadingAndTrailingSpaces(const std::string& str)
{
    std::string result;

    result = str;
    result.erase(0, str.find_first_not_of(' '));
    result.erase(result.find_last_not_of(' ') + 1);

    return (result);
}

void ConfigFileParser::readServerConfig(const std::string& configFileLine)
{
    Server server;
    std::string directive;
    const char* validServerDirectives[] = { "server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root", "location" };
    const int validServerDirectivesSize = sizeof(validServerDirectives) / sizeof(validServerDirectives[0]);

    std::set<std::string> validServerDirectivesSet(validServerDirectives, validServerDirectives + validServerDirectivesSize);

    directive = configFileLine.substr(0, configFileLine.find(' '));
    if (std::find(validServerDirectivesSet.begin(), validServerDirectivesSet.end(), directive) == validServerDirectivesSet.end())
        throw std::runtime_error("Error: Invalid server directive");

    m_configFile.servers.push_back(server);
}