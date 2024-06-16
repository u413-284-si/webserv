#include "ConfigFileParser.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv config.conf" << std::endl;	
		return (1);
	}	
	try
	{
		ConfigFileParser parser(argv[1]);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}