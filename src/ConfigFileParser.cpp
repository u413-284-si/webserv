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