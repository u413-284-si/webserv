#include "ConfigFile.hpp"
#include "ConfigFileParser.hpp"
#include "EpollWrapper.hpp"
#include "Log.hpp"
#include "LogData.hpp"
#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "ResponseBuilder.hpp"
#include "Server.hpp"
#include "signalHandler.hpp"
#include "SocketPolicy.hpp"
#include <cerrno>

ConfigFile createDummyConfig()
{
	Location location1;
	location1.path = "/";
	location1.root = "/workspaces/webserv";
	location1.indices.push_back("index.htm");

	ConfigServer serverConfig8080;
	serverConfig8080.locations.push_back(location1);
	serverConfig8080.host = "127.0.0.1";
	serverConfig8080.port = "8080";
	serverConfig8080.serverName = "root";

	Location location2;
	location2.path = "/";
	location2.root = "/workspaces/webserv/doc";
	location2.indices.push_back("index.htm");

	ConfigServer serverConfig8090;
	serverConfig8090.locations.push_back(location2);
	serverConfig8090.host = "127.0.0.1";
	serverConfig8090.port = "8090";
	serverConfig8090.serverName = "doc";

	ConfigServer serverConfig8090dupl;
	serverConfig8090dupl.locations.push_back(location2);
	serverConfig8090dupl.host = "127.0.0.1";
	serverConfig8090dupl.port = "8090";
	serverConfig8090dupl.serverName = "duplicate";

	ConfigFile configFile;
	configFile.servers.push_back(serverConfig8080);
	configFile.servers.push_back(serverConfig8090);
	configFile.servers.push_back(serverConfig8090dupl);

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

	if (std::signal(SIGINT, signalHandler) == SIG_ERR) {
		LOG_ERROR << "failed to set signal handler: " << std::strerror(errno);
		return 1;
	}

	ConfigFile configFile = createDummyConfig();

	weblog::initConsole(weblog::LevelDebug);
	try {
		EpollWrapper epollWrapper(10, -1);
		SocketPolicy socketPolicy;
		Server server(configFile, epollWrapper, socketPolicy);
		initVirtualServers(server, 10, server.getServerConfigs());
		ConfigFileParser parser;
		parser.parseConfigFile(argv[1]);
		runServer(server);
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
