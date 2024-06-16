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

ConfigFileParser::~ConfigFileParser()
{

}