#include "ConfigFile.hpp"

ConfigFile createTestConfigfile()
{
	Location location;
	location.path = "/";
	location.root = "/www/data";

	ConfigServer server;
	server.host = "127.0.0.1";
	server.port = "8080";
	server.locations.emplace_back(location);

	ConfigFile configFile;
	configFile.servers.emplace_back(server);

	return (configFile);
}
