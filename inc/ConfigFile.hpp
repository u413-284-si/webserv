#pragma once

/* ====== LIBRARIES ====== */

#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include "ConfigFile.hpp"

/* ====== DEFINITIONS ====== */

enum Method { MethodGet,
    MethodPost,
    MethodDelete,
    MethodCount
};

struct Location {
    std::vector<std::string> validLocationDirectives;
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
    std::vector<std::string> validServerDirectives;
    std::string serverName;
    std::string root;
	std::map<std::string, unsigned short> listen;
    unsigned long maxBodySize;
    std::map<unsigned short, std::string> errorPage;
	size_t locationIndex;
    std::vector<Location> locations;
};

struct ConfigFile {
    std::ifstream stream;
    std::string currentLine;
	size_t serverIndex;
    std::vector<ConfigServer> servers;
};

std::ostream& operator<<(std::ostream& ostream, const Location& location);
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer);
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile);
