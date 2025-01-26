#include "ConfigFileParser.hpp"
#include "Server.hpp"

/**
 * @brief Stringize the result of expansion of a macro argument
 *
 * If the macro DEFAULT_CONFIG_PATH is defined with -D at compile time the
 * literal value would be inserted. Since it has to be a string one would have
 * to put the value in escaped double quotes. This macro in combination with
 * STRINGIZE(s) stringize the defined value. First the macro gets expanded, and
 * then the expanded value gets stringized. One can simply redefine the path
 * with "-D DEFAULT_CONFIG_PATH=./new/path"
 * @sa https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
 */
#define XSTRINGIZE(s) STRINGIZE(s)

/**
 * @brief Converts macro argument into a string constant.
 *
 * Uses the '#' preprocessing operator. When a macro parameter is used with a
 * leading '#', the preprocessor replaces it with the literal text of the actual
 * argument
 */
#define STRINGIZE(s) #s

// clang-format off
#ifndef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_PATH ./config_files/trihard.conf
#endif
// clang-format on

int main(const int argc, const char* argv[])
{
	if (argc > 2) {
		std::cerr << "webserv: usage error: too many arguments provided" << '\n'
				  << "Usage: " << program_invocation_name << " [path to config file]" << '\n';
		return 1;
	}

	weblog::initConsole(weblog::LevelDebug);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const std::string configFilePath = (argc == 1) ? XSTRINGIZE(DEFAULT_CONFIG_PATH) : argv[1];
	LOG_INFO << "Config file path: " << configFilePath << '\n';

	if (!registerSignals())
		return 1;

	try {
		EpollWrapper epollWrapper(Server::s_maxEvents, Server::s_epollTimeout);
		FileSystemOps fileSystemOps;
		SocketOps socketOps;
		ProcessOps processOps;

		ConfigFileParser parser;
		ConfigFile configFile = parser.parseConfigFile(configFilePath);

		Server server(configFile, epollWrapper, fileSystemOps, socketOps, processOps);
		initVirtualServers(server, Server::s_backlog, server.getServerConfigs());
		runServer(server);
	} catch (std::exception& e) {
		LOG_ERROR << e.what();
		return 1;
	}
	return 0;
}
