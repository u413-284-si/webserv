#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include <stack>
#include <string>
#include <iostream>

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

	// Getter functions
	std::string getDirective(const std::string& line) const;
	std::string getValue(const std::string& line) const;

	// Reader functions	
    void readServerConfigLine(void);
    void readLocationConfigLine(void);
	void readServerDirectiveValue(const std::string& directive, const std::string &value);
	void readPort(const std::string& value);
	void readIpAddress(const std::string &value);
	void readRootPath(int block, const std::string& value);

	// Checker functions
    bool isBracketOpen(const std::string& configFilePath);
	bool isSemicolonMissing(const std::string& line) const;
	bool isDirectiveValid(const std::string& directive, int block) const;
	bool isIpAddressValid(const std::string& ip) const;
	bool isPortValid(const std::string& port) const;

	// Helper functions
    void readAndTrimLine(void);
    void removeLeadingAndTrailingSpaces(std::string &line) const;
};
