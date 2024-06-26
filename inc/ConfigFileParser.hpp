#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include <stack>

/* ====== DEFINITIONS ====== */

#define SERVER 1
#define LOCATION 2

/* ====== CLASS DECLARATION ====== */

class ConfigFileParser {
public:
    ConfigFileParser(void);
    ~ConfigFileParser();

	const ConfigFile& parseConfigFile(const std::string& configFilePath);

private:
    ConfigFile m_configFile;
    std::stack<char> m_brackets;

	
    void readServerConfig(size_t index);
    void readLocationConfig(size_t index);
	static bool isDirectiveValid(const std::string& directive, int block);
    bool isBracketOpen(const std::string& configFilePath);

	// Helper functions
    bool readAndTrimLine(void);
    void removeLeadingAndTrailingSpaces();
};
