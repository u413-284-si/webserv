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
	ostream << "Path: " << location.getPath() << '\n';
	ostream << "Root: " << location.getRoot() << '\n';
	ostream << "Index:\n";
	std::vector<std::string> indices = location.getIndices();
	for (std::vector<std::string>::const_iterator it = indices.begin(), end = indices.end(); it != end; ++it)
		ostream << *it << '\n';
	ostream << "CGI extension: " << location.getCgiExt() << '\n';
	ostream << "CGI path: " << location.getCgiPath() << '\n';
	ostream << "Autoindex: " << location.getIsAutoindex() << '\n';
	ostream << "Max body size: " << location.getMaxBodySize() << '\n';
	ostream << "Error pages:\n";
	std::map<unsigned short, std::string> errorPages = location.getErrorPages();
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(), end = errorPages.end();
		 it != end; ++it)
		ostream << it->first << ": " << it->second << '\n';
	ostream << "Allowed methods:\n";
	std::cout << "Get: " << location.getAllowedMethod(MethodGet) << '\n';
	std::cout << "Post: " << location.getAllowedMethod(MethodPost) << '\n';
	std::cout << "Delete: " << location.getAllowedMethod(MethodDelete) << '\n';
	ostream << "Returns:\n";
	std::map<unsigned short, std::string> returns = location.getReturns();
	for (std::map<unsigned short, std::string>::const_iterator it = returns.begin(); it != returns.end(); ++it) {
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
	ostream << "Server: " << configServer.getServerName() << '\n';
	ostream << "Root: " << configServer.getRoot() << '\n';
	ostream << "Listen:\n";
	std::map<std::string, unsigned short> listen = configServer.getListen();
	for (std::map<std::string, unsigned short>::const_iterator it = listen.begin(); it != listen.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Max body size: " << configServer.getMaxBodySize() << '\n';
	ostream << "Error pages:\n";
	std::map<unsigned short, std::string> errorPages = configServer.getErrorPages();
	for (std::map<unsigned short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Locations:\n";
	std::vector<Location> locations = configServer.getLocations();
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
