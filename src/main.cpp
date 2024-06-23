#include "Server.hpp"
#include "ConfigFileParser.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\nexpected: ";
		std::cerr << argv[0] << " <config file>\n";
		return 1;
	}
	(void)argv;
	try{
		Server	webserv;
		ConfigFileParser configFileParser(argv[1]);

		webserv.run();
	}
	catch (std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
