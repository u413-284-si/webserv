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
    bool readAndTrimLine(void);
    void checkBrackets(const std::string& configFilePath);
    void readServerConfig(size_t index);
    void readLocationConfig(size_t index);
    void removeLeadingAndTrailingSpaces();
};