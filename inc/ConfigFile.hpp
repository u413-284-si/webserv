#pragma once

#include <iostream>
#include <map>
#include <vector>

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

struct ServerConfig {
    std::vector<std::string> validServerDirectives;
    std::string serverName;
    std::string host;
    std::string root;
    unsigned short port;
    unsigned long maxBodySize;
    std::map<unsigned short, std::string> errorPage;
    std::vector<Location> locations;
};

struct ConfigFile {
    std::vector<ServerConfig> serverConfigs;
};