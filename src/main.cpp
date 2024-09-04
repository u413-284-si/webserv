#include "ConfigFileParser.hpp"
#include "Log.hpp"
#include "Server.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: " << program_invocation_name << " <config file>\n";
		return 1;
	}

	// Return of SIG_ERR in case of failure of std::signal() is specified in man page.
	// This line generates two errors: C-Style cast and downcast impacts.
	// 1. Reinterpret_cast would generate another error, so just deactivated.
	// 2. There is no performance lost by downcasting, since it's a simple check.
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
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
