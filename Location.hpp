#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <map>
#include <vector>

class Location {
public:
    Location();
    ~Location();

private:
    std::string m_path;
    std::string m_root;
    std::string m_index;
    bool m_autoindex;
    std::vector<std::string> m_allowed_methods;
    std::map<unsigned short, std::string> m_return;
};

#endif