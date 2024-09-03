#include "ConfigFile.hpp"
#include "ConfigFileParser.hpp"
#include "EpollWrapper.hpp"
#include "Log.hpp"
#include "LogData.hpp"
#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "ResponseBuilder.hpp"
#include "Server.hpp"
#include "SocketPolicy.hpp"
#include "signalHandler.hpp"
#include <cerrno>

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: " << program_invocation_name << " <config file>\n";
		return 1;
	}

	if (std::signal(SIGINT, signalHandler) == SIG_ERR) {
		std::cerr << "error: failed to set signal handler: " << std::strerror(errno);
		return 1;
	}

	weblog::initConsole(weblog::LevelDebug);

	try {
		EpollWrapper epollWrapper(10, -1);
		SocketPolicy socketPolicy;

		ConfigFileParser parser;
		ConfigFile configFile = parser.parseConfigFile(argv[1]);

		Server server(configFile, epollWrapper, socketPolicy);
		initVirtualServers(server, 10, server.getServerConfigs());
		runServer(server);
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
