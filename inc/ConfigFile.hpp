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
    const std::string m_path;
    const std::string m_root;
    const std::string m_index;
    const std::string m_cgiExt;
    const std::string m_cgiPath;
    const bool m_autoindex;
    bool m_allowed_methods[MethodCount];
    std::map<unsigned short, std::string> m_return;
};

struct ServerConfig {
    const std::string m_serverName;
    const std::string m_host;
    const unsigned short m_port;
    const unsigned long m_maxBodySize;
    std::map<unsigned short, std::string> m_errorPage;
    std::vector<Location> m_locations;
};

struct ConfigFile {
    std::vector<ServerConfig> m_serverConfigs;
};
