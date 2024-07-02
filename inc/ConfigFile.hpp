#pragma once

/* ====== LIBRARIES ====== */

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

/* ====== DEFINITIONS ====== */

enum Method { MethodGet,
    MethodPost,
    MethodDelete,
    MethodCount
};

struct LimitExcept {
    std::vector<std::string> validLimitExceptDirectives;
    bool allowedMethods[MethodCount];
    std::string allow;
    std::string deny;
};

struct Location {
    std::vector<std::string> validLocationDirectives;
    std::string path;
    std::string root;
    std::string index;
    std::string cgiExt;
    std::string cgiPath;
    bool isAutoindex;
    LimitExcept limitExcept;
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
