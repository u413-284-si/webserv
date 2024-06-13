#include "LogOutputterConsole.hpp"

void LogOutputterConsole::log(const LogLevel level, const std::string& message)
{
	if (level >= LevelWarn)
		std::cerr << message;
	else
		std::clog << message;
}
