#pragma once

#include "ConfigFile.hpp"

class ConfigFileParser {
public:
    ConfigFileParser(std::string config_file);
    ~ConfigFileParser();

private:
    struct ConfigFile m_configFile;
};
