#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <stack>
#include <string>

/* ====== DEFINITIONS ====== */

enum Block { ServerBlock, LocationBlock };

/* ====== CLASS DECLARATION ====== */

class ConfigFileParser {
public:
	const ConfigFile& parseConfigFile(const std::string& configFilePath);

	ConfigFileParser(void);
	~ConfigFileParser();

private:
	ConfigFileParser(const ConfigFileParser& ref);
	ConfigFileParser& operator=(const ConfigFileParser& ref);

	ConfigFile m_configFile;
	size_t m_serverIndex;
	std::vector<std::string> m_validServerDirectives;
	size_t m_locationIndex;
	std::vector<std::string> m_validLocationDirectives;

	// Getter functions
	std::string getDirective(const std::string& line) const;
	std::string getValue(const std::string& line) const;

	// Reader functions
	void readServerConfigLine(void);
	void readLocationConfigLine(void);
	void readServerDirectiveValue(const std::string& directive, const std::string& value);
	void readSocket(const std::string& value);
	void readRootPath(int block, const std::string& value);

	// Checker functions
	static bool isBracketOpen(const std::string& configFilePath);
	bool isSemicolonMissing(const std::string& line) const;
	bool isDirectiveValid(const std::string& directive, int block) const;
	bool isIpAddressValid(const std::string& ipAddress) const;
	bool isPortValid(const std::string& port) const;

	// Helper functions
	void readAndTrimLine(void);
	void processRemainingLine(std::string& line) const;
};
