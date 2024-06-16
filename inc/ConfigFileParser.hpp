#pragma once

#include "ConfigFile.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include <stack>

class ConfigFileParser {
public:
    ConfigFileParser(const std::string& configFilePath);
    ~ConfigFileParser();

private:
    ConfigFile m_configFile;
    std::stack<char> m_brackets;
    void checkBrackets(const std::string& configFileLine);
    void readServerConfig(const std::string& configFileLine);
    std::string removeLeadingAndTrailingSpaces(const std::string& str);
};