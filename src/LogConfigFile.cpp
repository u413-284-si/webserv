#include <iostream>
#include <string>
#include <vector>

#include "ConfigFile.hpp"

/**
 * @brief Overload << operator to append a Location.
 *
 * @param ostream The output stream.
 * @param location The Location object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const Location& location)
{
	ostream << "Path: " << location.path << '\n';
	ostream << "Root: " << location.root << '\n';
	ostream << "Index:\n";
	std::vector<std::string> indices = location.indices;
	for (std::vector<std::string>::const_iterator it = indices.begin(), end = indices.end(); it != end; ++it)
		ostream << *it << '\n';
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.isAutoindex << '\n';
	ostream << "Max body size: " << location.maxBodySize << '\n';
	ostream << "Error pages:\n";
	std::map<unsigned short, std::string> errorPages = location.errorPage;
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(), end = errorPages.end();
		 it != end; ++it)
		ostream << it->first << ": " << it->second << '\n';
	ostream << "Allowed methods:\n";
	std::cout << "Get: " << location.allowedMethods[MethodGet] << '\n';
	std::cout << "Post: " << location.allowedMethods[MethodPost] << '\n';
	std::cout << "Delete: " << location.allowedMethods[MethodDelete] << '\n';
	ostream << "Returns:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = location.returns.begin();
		 it != location.returns.end(); ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
	return ostream;
}

/**
 * @brief Overload << operator to append a ServerConfig.
 *
 * @param ostream The output stream.
 * @param serverConfig The ServerConfig object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer)
{
	ostream << "Server: " << configServer.serverName << '\n';
	ostream << "Root: " << configServer.root << '\n';
	ostream << "Listen:\n";
	std::map<std::string, std::string> listen = configServer.listen;
	for (std::map<std::string, std::string>::const_iterator it = listen.begin(); it != listen.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Max body size: " << configServer.maxBodySize << '\n';
	ostream << "Error pages:\n";
	std::map<unsigned short, std::string> errorPages = configServer.errorPage;
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Locations:\n";
	std::vector<Location> locations = configServer.locations;
	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
		ostream << *it;
	return ostream;
}

/**
 * @brief Overload << operator to append a ConfigFile.
 *
 * @param ostream The output stream.
 * @param configFile The ConfigFile object.
 * @return std::ostream& The output stream.
 */
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile)
{
	ostream << "Config file:\n";
	for (std::vector<ConfigServer>::const_iterator it = configFile.servers.begin(); it != configFile.servers.end();
		 ++it)
		ostream << *it;
	return ostream;
}
