#include "test_helpers.hpp"

ConfigFile createTestConfigfile()
{
	ConfigServer server;
	server.host = "127.0.0.1";
	server.port = "8080";

	ConfigFile configFile;
	configFile.servers.emplace_back(server);

	return (configFile);
}
