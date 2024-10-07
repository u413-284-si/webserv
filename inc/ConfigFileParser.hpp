#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

enum Block { HttpBlock, ServerBlock, LocationBlock };

struct LocationContent {
	std::string content;
};

struct ServerContent {
	std::string content;
	std::vector<LocationContent> locations;
};

/* ====== CLASS DECLARATION ====== */

class ConfigFileParser {
public:
	const ConfigFile& parseConfigFile(const std::string& configFilePath);

	ConfigFileParser(void);

private:
	ConfigFile m_configFile;
	std::string m_configFileContent;
	std::vector<ServerContent> m_serversContent;
	std::stringstream m_stream;
	std::string m_currentLine;
	size_t m_configFileIndex;
	size_t m_serverIndex;
	std::vector<std::string> m_validServerDirectives;
	size_t m_locationIndex;
	std::vector<std::string> m_validLocationDirectives;
	static const char* const whitespace;

	// Getter functions
	std::string getDirective(void) const;
	std::string getValue(void) const;

	// Reader functions
	void readServerBlock();
	void readLocationBlock(ServerContent& serverContent);
	void processServerContent(const ServerContent& serverContent);
	void processLocationContent(const LocationContent& locationContent);
	void readServerConfigLine(void);
	void readLocationConfigLine(void);
	void readServerDirectiveValue(const std::string& directive, const std::string& value);
	void readSocket(const std::string& value);
	void readRootPath(Block block, const std::string& value);

	// Checker functions
	bool isBracketOpen(const std::string& configFileContent);
	bool isKeyword(const std::string& keyword, size_t startIndex) const;
	bool isValidBlockBeginn(Block block);
	bool isDirectiveValid(const std::string& directive, Block block) const;
	static bool isIpAddressValid(const std::string& ipAddress);
	static bool isPortValid(const std::string& port);

	// Helper functions
	std::string convertBlockToString(Block block) const;
	void skipBlockBegin(Block block);
	void readAndTrimLine(void);
};
