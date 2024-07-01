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

	
    void readServerConfigLine(void);
    void readLocationConfigLine(void);
	void readDirectiveValue(const std::string& directive);

	// Checker functions
    bool isBracketOpen(const std::string& configFilePath);
	bool isDirectiveValid(const std::string& directive, int block) const;
	bool isSemicolonAtEnd(void) const;
	bool isSemicolonCountOne(void) const;

	bool isListenValueValid(const std::string& directive);

	// Helper functions
    bool readAndTrimLine(void);
    void removeLeadingAndTrailingSpaces();
};
