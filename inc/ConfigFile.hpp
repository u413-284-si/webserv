#pragma once

/* ====== LIBRARIES ====== */

#include "Method.hpp"
#include "StatusCode.hpp"
#include "constants.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

/* ====== DEFINITIONS ====== */

struct Location {

public:
	Location(void);

	std::string path;
	std::string root;
	std::string alias;
	std::vector<std::string> indices;
	std::string cgiExt;
	std::string cgiPath;
	bool hasAutoindex;
	unsigned long maxBodySize;
	std::map<statusCode, std::string> errorPage;
	bool allowMethods[MethodCount];
	std::pair<statusCode, std::string> returns;
	std::string alias;
};

struct ConfigServer {

public:
	ConfigServer(void);

	std::string serverName;
	std::string root;
	std::string host;
	std::string port;
	size_t maxBodySize;
	std::map<statusCode, std::string> errorPage;
	std::vector<Location> locations;
};

struct ConfigFile {
	std::vector<ConfigServer> servers;
};

ConfigFile createDummyConfig();

std::ostream& operator<<(std::ostream& ostream, const Location& location);
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer);
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile);
