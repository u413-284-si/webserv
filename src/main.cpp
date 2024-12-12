#include "ConfigFileParser.hpp"
#include "Server.hpp"

#ifndef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_PATH "./config_files/standard_config.conf"
#endif

int main(int argc, char** argv)
{
	if (argc > 2) {
		std::cerr << "webserv: usage error: too many arguments provided" << '\n'
				  << "Usage: " << program_invocation_name << " [path to config file]" << '\n';
		return 1;
	}

	//NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const std::string configFilePath = (argc == 1) ? DEFAULT_CONFIG_PATH : argv[1];
	std::cout << "Config file path: " << configFilePath << '\n';

	weblog::initConsole(weblog::LevelDebug);

	if (!registerSignals()) {
		return 1;
	}

	try {
		EpollWrapper epollWrapper(10, -1);
		SocketPolicy socketPolicy;
		ProcessOps processOps;

		ConfigFileParser parser;
		ConfigFile configFile = parser.parseConfigFile(configFilePath);

		configFile = createDummyConfig();

		Server server(configFile, epollWrapper, socketPolicy, processOps);
		initVirtualServers(server, 10, server.getServerConfigs());
		runServer(server);
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
