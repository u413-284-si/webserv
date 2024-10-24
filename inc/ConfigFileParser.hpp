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

private:
	ConfigFile m_configFile;
	std::ifstream m_stream;
	std::string m_currentLine;
	size_t m_serverIndex;
	std::vector<std::string> m_validServerDirectives;
	size_t m_locationIndex;
	std::vector<std::string> m_validLocationDirectives;
	static const char* const whitespace;

	// Getter functions
	std::string getDirective(const std::string& line) const;
	std::string getValue(const std::string& line) const;

	// Reader functions
	void readServerConfigLine(void);
	void readLocationConfigLine(void);
	void readServerDirectiveValue(const std::string& directive, const std::string& value);
	void readSocket(const std::string& value);
	void readRootPath(Block block, const std::string& value);

	// Checker functions
	static bool isBracketOpen(const std::string& configFilePath);
	bool isSemicolonMissing(const std::string& line) const;
	bool isDirectiveValid(const std::string& directive, Block block) const;

	// Helper functions
	void readAndTrimLine(void);
	void processRemainingLine(std::string& line);
};
