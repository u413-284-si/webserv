#include <iostream>

#include "ConfigFile.hpp"

std::ostream& operator<<(std::ostream& ostream, const Location& location)
{
	ostream << "Path: " << location.path << '\n';
	ostream << "Root: " << location.root << '\n';
	ostream << "Index: " << location.index << '\n';
	ostream << "CGI extension: " << location.cgiExt << '\n';
	ostream << "CGI path: " << location.cgiPath << '\n';
	ostream << "Autoindex: " << location.isAutoindex << '\n';
	ostream << "LimitExcept:\n";
	ostream << "  Allow: " << location.limitExcept.allow << '\n';
	ostream << "  Deny: " << location.limitExcept.deny << '\n';
	ostream << "  Allowed methods:\n";
	ostream << "Returns:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = location.returns.begin();
		 it != location.returns.end(); ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
	return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const ServerConfig& serverConfig)
{
	ostream << "Server: " << serverConfig.serverName << '\n';
	ostream << "Host: " << serverConfig.host << '\n';
	ostream << "Port: " << serverConfig.port << '\n';
	ostream << "Max body size: " << serverConfig.maxBodySize << '\n';
	ostream << "Error pages:\n";
	for (std::map<unsigned short, std::string>::const_iterator it = serverConfig.errorPage.begin();
		 it != serverConfig.errorPage.end(); ++it) {
		ostream << "  " << it->first << ": " << it->second << '\n';
	}
	ostream << "Locations:\n";
	for (std::vector<Location>::const_iterator it = serverConfig.locations.begin(); it != serverConfig.locations.end();
		 ++it) {
		ostream << *it;
	}
	return ostream;
}

/**
 * @brief Overload << operator to append a ConfigFile.
 *
 * @return LogData& The LogData object.
 */
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile)
{
	ostream << "Config file:\n";
	for (std::vector<ServerConfig>::const_iterator it = configFile.serverConfigs.begin();
		 it != configFile.serverConfigs.end(); ++it) {
		ostream << *it;
	}
	return ostream;
}
