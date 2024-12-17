#pragma once

/* ====== LIBRARIES ====== */

#include "Method.hpp"
#include "StatusCode.hpp"
#include "constants.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

struct Location {

public:
	Location(void);

	std::string path;
	std::string root;
	std::vector<std::string> indices;
	std::string cgiExt;
	std::string cgiPath;
	bool isAutoindex;
	unsigned long maxBodySize;
	std::map<statusCode, std::string> errorPage;
	bool allowedMethods[MethodCount];
	std::map<statusCode, std::string> returns;
};

struct ConfigServer {

public:
	ConfigServer(void);

	std::string serverName;
	std::string root;
	// std::map<std::string, std::string> listen;
	std::string host;
	std::string port;
	unsigned long maxBodySize;
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
