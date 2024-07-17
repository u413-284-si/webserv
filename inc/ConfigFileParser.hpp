#pragma once

#include "ConfigFile.hpp"

class ConfigFileParser {
public:
	ConfigFileParser(std::string configFile);
	~ConfigFileParser();

private:
	ConfigFile m_configFile;
};
