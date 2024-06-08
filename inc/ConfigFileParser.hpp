#pragma once

#include "ConfigFile.hpp"

class ConfigFileParser {
public:
    ConfigFileParser(std::string configFile);
    ~ConfigFileParser();

private:
    struct ConfigFile m_configFile;
};
