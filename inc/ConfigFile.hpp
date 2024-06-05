#pragma once

#include "ServerConfig.hpp"

class ConfigFile {
public:
    ConfigFile();
    ~ConfigFile();

private:
    std::vector<ServerConfig> m_servers_config;
};

#endif