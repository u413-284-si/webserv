#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "Server.hpp"
#include "Log.hpp"
#include "ConfigFile.hpp"
#include <cerrno>
#include "Dispatcher.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: ";
		std::cerr << program_invocation_name << " <config file>\n";
		return 1;
	}
	static_cast<void>(argv);

	Location location = {};
	location.path = "/";
	location.root = "/workspaces/webserv";
	location.index = "index.html";

	ServerConfig serverConfig;
	serverConfig.locations.push_back(location);
	serverConfig.port = 8080;
	serverConfig.host = "localhost";

	ConfigFile configFile;
	configFile.serverConfigs.push_back(serverConfig);

	weblog::initConsole(weblog::LevelDebug);
	try{
		Dispatcher dispatcher(-1);
		LOG_DEBUG << "Add new server";
		dispatcher.addListeningEndpoint("*", 10, "8080");
		dispatcher.handleEvents();
	}
	catch (std::exception& e){
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
