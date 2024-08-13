#pragma once

/* ====== LIBRARIES ====== */

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

enum Method { MethodGet, MethodPost, MethodDelete, MethodCount };

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
	std::map<unsigned short, std::string> errorPage;
	bool allowedMethods[MethodCount];
	std::map<unsigned short, std::string> returns;
};

struct ConfigServer {

public:
	ConfigServer(void);

	std::string serverName;
	std::string root;
	std::map<std::string, std::string> listen;
	unsigned long maxBodySize;
	std::map<unsigned short, std::string> errorPage;
	std::vector<Location> locations;
};

struct ConfigFile {
	std::ifstream stream;
	std::string currentLine;
	std::vector<ConfigServer> servers;
};

std::ostream& operator<<(std::ostream& ostream, const Location& location);
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer);
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile);
