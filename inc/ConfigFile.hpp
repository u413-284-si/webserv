#pragma once

#include <iostream>
#include <map>
#include <vector>

enum Method { MethodGet, MethodPost, MethodDelete, MethodCount };

struct LimitExcept {
	bool allowedMethods[MethodCount];
	std::string allow;
	std::string deny;
};

struct Location {
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

