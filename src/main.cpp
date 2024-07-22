#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "Server.hpp"
#include "Log.hpp"
#include "ConfigFile.hpp"
#include "Dispatcher.hpp"

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
		Dispatcher dispatcher(-1);
		LOG_DEBUG << "Add new server";
		dispatcher.initServer("*", 10, "8080");
		LOG_INFO << "Dispatcher started";
		dispatcher.handleEvents();
	}
	catch (std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
