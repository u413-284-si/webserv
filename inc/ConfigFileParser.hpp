#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"
#include "error.hpp"
#include "limits"
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

struct ServerBlockConfig {
	std::string serverBlockContent;
	std::vector<std::string> locationBlocksContent;
};

/* ====== CLASS DECLARATION ====== */

class ConfigFileParser {
public:
	const ConfigFile& parseConfigFile(const std::string& configFilePath);

	ConfigFileParser(void);

private:
	ConfigFile m_configFile;
	std::string m_configFileContent;
	std::vector<ServerBlockConfig> m_serverBlocksConfig;
	std::string m_currentLine;
	size_t m_configFileIndex;
	size_t m_contentIndex;
	size_t m_serverIndex;
	std::vector<std::string> m_validServerDirectives;
	size_t m_locationIndex;
	std::vector<std::string> m_validLocationDirectives;
	bool m_isDefaultLocationDefined;
	static const char* const s_whitespace;
	static const char* const s_number;

	// Checker functions
	bool isBracketOpen(void);
	bool isSemicolonMissing(const std::string& content) const;
	bool isKeyword(const std::string& keyword, size_t startIndex) const;
	bool isValidBlockBeginn(Block block);
	bool isDirectiveValid(const std::string& directive, Block block) const;
	bool isLocationDuplicate(void) const;
	static bool isIpAddressValid(const std::string& ipAddress);
	static bool isPortValid(const std::string& port);

	// Reader functions
	bool readAndTrimLine(const std::string& content, char delimiter);
	void readServerBlock();
	void readLocationBlock(ServerBlockConfig& serverBlockConfig);
	void processServerContent(const ServerBlockConfig& serverBlockConfig);
	void processLocationContent(const std::string& locationBlockContent);
	void readServerConfigLine(void);
	void readLocationConfigLine(void);
	void readServerDirectiveValue(const std::string& directive, const std::string& value);
	void readLocationDirectiveValue(const std::string& directive, const std::string& value);
	void readServerName(const std::string& serverName);
	void readListen(const std::string& value);
	void readRootPath(const Block& block, std::string rootPath);
	void readAliasPath(const std::string& aliasPath);
	void readMaxBodySize(const Block& block, const std::string& maxBodySize);
	void readAutoIndex(const std::string& autoindex);
	void readAllowMethods(const std::string& allowMethods);
	void readErrorPage(const Block& block, const std::string& errorPage);
	void readReturns(const std::string& returns);
	void readCGIExtension(const std::string& extension);
	void readCGIPath(const std::string& path);
	void readIndex(const std::string& indices);
	void readLocationBlockPath(void);

	// Helper functions
	std::string getDirective(void) const;
	std::string getValue(void) const;
	std::string convertBlockToString(Block block) const;
	void removeEnclosingDoubleQuotes(std::string& str);
	void skipBlockBegin(Block block);
	void skipLocationBlockPath(size_t& index);
};
