#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "Server.hpp"
#include "ConfigFileParser.hpp"
#include "Log.hpp"
#include "ConfigFile.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: ";
		std::cerr << argv[0] << " <config file>\n";
		return 1;
	}
	static_cast<void>(argv);
	weblog::initConsole(weblog::LevelDebug);
	try{
		Server	webserv;
		ConfigFileParser configFileParser;
		configFileParser.parseConfigFile(argv[1]);
		LOG_INFO << "Starting server";
		webserv.run();
	}
	catch (std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
