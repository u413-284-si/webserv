#include "ConfigFile.hpp"
#include "Log.hpp"
#include "LogData.hpp"
#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "ResponseBuilder.hpp"
#include "Server.hpp"
#include <cerrno>

ConfigFile createDummyConfig()
{
	Location location1 = {};
	location1.path = "/";
	location1.root = "/workspaces/webserv";
	location1.index = "index.html";

	ServerConfig serverConfig8080;
	serverConfig8080.locations.push_back(location1);
	serverConfig8080.port = 8080;
	serverConfig8080.host = "127.0.0.1";
	serverConfig8080.serverName = "root";

	Location location2 = {};
	location2.path = "/";
	location2.root = "/workspaces/webserv/doc";
	location2.index = "index.html";

	ServerConfig serverConfig8090;
	serverConfig8090.locations.push_back(location2);
	serverConfig8090.port = 8090;
	serverConfig8090.host = "127.0.0.1";
	serverConfig8090.serverName = "doc";

	ServerConfig serverConfig8090dupl;
	serverConfig8090dupl.locations.push_back(location2);
	serverConfig8090dupl.port = 8090;
	serverConfig8090dupl.host = "127.0.0.1";
	serverConfig8090dupl.serverName = "duplicate";

	ConfigFile configFile;
	configFile.serverConfigs.push_back(serverConfig8080);
	configFile.serverConfigs.push_back(serverConfig8090);
	configFile.serverConfigs.push_back(serverConfig8090dupl);

	return configFile;
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: ";
		std::cerr << program_invocation_name << " <config file>\n";
		return 1;
	}
	static_cast<void>(argv);

	ConfigFile configFile = createDummyConfig();
	
	weblog::initConsole(weblog::LevelDebug);
	try {
		Server server(configFile, -1);
		server.run();
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
