#pragma once

#include "ConfigFile.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include <stack>

#define SERVER 1
#define LOCATION 2

class ConfigFileParser {
public:
    ConfigFileParser(void);
    ~ConfigFileParser();

	const ConfigFile& parseConfigFile(const std::string& configFilePath);

private:
    ConfigFile m_configFile;
    std::stack<char> m_brackets;
    bool readAndTrimLine(void);
    bool isBracketOpen(const std::string& configFilePath);
    void readServerConfig(size_t index);
    void readLocationConfig(size_t index);
    void removeLeadingAndTrailingSpaces();
	size_t countChars(const std::string& line, char c);
};
