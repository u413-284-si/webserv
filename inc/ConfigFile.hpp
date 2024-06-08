#pragma once

#include <iostream>
#include <map>
#include <vector>

enum Method { MethodGet,
    MethodPost,
    MethodPut,
    MethodDelete,
    MethodCount };

struct Location {
    std::string path;
    std::string root;
    std::string index;
    std::string cgiExt;
    std::string cgiPath;
    bool autoindex;
    bool allowed_methods[MethodCount];
    std::map<unsigned short, std::string> returns;
};

struct ServerConfig {
    std::string serverName;
    std::string host;
    unsigned short port;
    unsigned long maxBodySize;
    std::map<unsigned short, std::string> errorPage;
    std::vector<Location> locations;
};

struct ConfigFile {
    std::vector<ServerConfig> serverConfigs;
};
