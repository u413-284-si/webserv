#include "Server.hpp"

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "error: arguments invalid\n";
		return 1;
	}
	(void)argv;
	try{
		Server	webserv;
		webserv.run();
	}
	catch (std::exception& e){
		std::cerr << "error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
