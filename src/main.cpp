#include "ConfigFileParser.hpp"
#include "Server.hpp"

/**
 * @brief Stringize the result of expansion of a macro argument
 *
 * If the macro DEFAULT_CONFIG_PATH is defined with -D at compile time the literal value would be inserted. For it to be
 * a string the user would have to escape double quotes. These two macro stringize the defined value. The user can
 * redefine the path as "-D DEFAULT_CONFIG_PATH=./new/path"
 * @sa https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
 */
#define xstr(s) str(s)

/**
 * @brief Converts macro argument into a string constant.
 *
 * Uses the '#' preprocessing operator. When a macro parameter is used with a leading '#', the preprocessor replaces it
 * with the literal text of the actual argument
 */
#define str(s) #s

#ifndef DEFAULT_CONFIG_PATH
#define DEFAULT_CONFIG_PATH ./ config_files / standard_config.conf
#endif

int main(const int argc, const char* argv[])
{
	if (argc > 2) {
		std::cerr << "webserv: usage error: too many arguments provided" << '\n'
				  << "Usage: " << program_invocation_name << " [path to config file]" << '\n';
		return 1;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
	const std::string configFilePath = (argc == 1) ? xstr(DEFAULT_CONFIG_PATH) : argv[1];
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
