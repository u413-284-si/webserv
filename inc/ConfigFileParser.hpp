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

	// Initializer functions
	void initializeConfigServer(ConfigServer &configServer);
	void initializeLocation(Location &location);

	// Reader functions	
    void readServerConfigLine(void);
    void readLocationConfigLine(void);
	void readDirectiveValue(const std::string& directive);

	// Checker functions
    bool isBracketOpen(const std::string& configFilePath);
	bool isDirectiveValid(const std::string& directive, int block) const;
	bool isSemicolonAtEnd(void) const;
	bool isSemicolonCountOne(void) const;
	bool isListenIpValid(void);
	bool isListenPortValid(void);

	// Helper functions
    void readAndTrimLine(void);
    void removeLeadingAndTrailingSpaces();
};
