#include "ConfigFileParser.hpp"


ConfigFileParser::ConfigFileParser(const std::string& configFilePath)
{
	std::ifstream inputConfigFile;
	std::string configFileLine;

	inputConfigFile.open(configFilePath.c_str());
	if (!inputConfigFile)
		throw std::runtime_error("Error: Failed to open config file");
	else if (inputConfigFile.peek() == std::ifstream::traits_type::eof())
		throw std::runtime_error("Error: Config file is empty");

	getline(inputConfigFile, configFileLine);
	if (removeLeadingAndTrailingSpaces(configFileLine) != "http {")
		throw std::runtime_error("Error: Config file does not start with 'http {'");
	checkBrackets(configFileLine);
}

ConfigFileParser::~ConfigFileParser(){}

void ConfigFileParser::checkBrackets(const std::string &configFileLine)
{
	for (std::string::const_iterator it = configFileLine.begin(); it != configFileLine.end(); it++)
	{
		if (*it == '{')
			m_brackets.push('{');
		else if (*it == '}')
			m_brackets.pop();
	}

}

std::string ConfigFileParser::removeLeadingAndTrailingSpaces(const std::string &str)
{
	std::string result;
	
	result = str;
	result.erase(0, str.find_first_not_of(' '));
	result.erase(result.find_last_not_of(' ') + 1);

	return (result);
}

void ConfigFileParser::readServerConfig(const std::string &configFileLine)
{
	ServerConfig serverConfig;
	std::string directive;
	const char* validServerDirectives[] = {"server_name", "listen", "host", "client_max_body_size", "error_page", "location", "root", "location"};
	const int validServerDirectivesSize = sizeof(validServerDirectives) / sizeof(validServerDirectives[0]);
	
	std::set<std::string> validServerDirectivesSet(validServerDirectives, validServerDirectives + validServerDirectivesSize);

	directive = configFileLine.substr(0, configFileLine.find(' '));
	if (std::find(validServerDirectivesSet.begin(), validServerDirectivesSet.end(), directive) == validServerDirectivesSet.end())
		throw std::runtime_error("Error: Invalid server directive");

	m_configFile.serverConfigs.push_back(serverConfig);
}