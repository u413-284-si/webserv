#include "ConfigFileParser.hpp"
#include "ProcessOps.hpp"
#include "Server.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: " << program_invocation_name << " <config file>\n";
		return 1;
	}

	weblog::initConsole(weblog::LevelDebug);

	if (!registerSignals()) {
		return 1;
	}

	try {
		EpollWrapper epollWrapper(10, -1);
		SocketPolicy socketPolicy;
		ProcessOps processOps;

		ConfigFileParser parser;
		ConfigFile configFile = parser.parseConfigFile(argv[1]);

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
