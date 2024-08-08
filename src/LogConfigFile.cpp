#include <iostream>
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
	for (std::vector<std::string>::const_iterator it = location.indices.begin(), end = location.indices.end(); it != end; ++it)
		ostream << *it << '\n';	
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.isAutoindex << '\n';
	ostream << "Max body size: " << location.maxBodySize << '\n';
	ostream << "Error pages:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = location.errorPage.begin(), end = location.errorPage.end(); it != end; ++it)
		ostream << it->first << ": " << it->second << '\n';	
	ostream << "Allowed methods:\n";
	for (int i = 0; i < MethodCount - 1; ++i)
		ostream << "  " << location.allowedMethods[i] << '\n';	
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
	for (std::map<std::string, unsigned short>::const_iterator it = configServer.listen.begin(); it != configServer.listen.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Max body size: " << configServer.maxBodySize << '\n';
	ostream << "Error pages:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = configServer.errorPage.begin();
		 it != configServer.errorPage.end(); ++it)
		ostream << "  " << it->first << ": " << it->second << '\n';
	ostream << "Locations:\n";
	for (std::vector<Location>::const_iterator it = configServer.locations.begin(); it != configServer.locations.end();
		 ++it)
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
	for (std::vector<ConfigServer>::const_iterator it = configFile.servers.begin();
		 it != configFile.servers.end(); ++it)
		ostream << *it;
	return ostream;
}
