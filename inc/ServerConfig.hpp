#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Location.hpp"
#include <iostream>
#include <map>
#include <vector>

class ServerConfig {
public:
    ServerConfig();
    ~ServerConfig();

private:
    std::string m_server_name;
    std::string m_host;
    std::string m_index;
    std::string m_root;
    unsigned short m_port;
    unsigned long m_max_body_size;
    std::map<unsigned short, std::string> m_error_page;
    std::vector<Location> m_locations;
};

#endif