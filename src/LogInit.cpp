#include "Log.hpp"

namespace weblog
{

Logger& initFile(LogLevel level, const char * filename)
{
	static LogOutputterFile outputter(filename);
	return Logger::init(level, &outputter);
}

Logger& initConsole(LogLevel level)
{
	static LogOutputterConsole outputter;
	return Logger::init(level, &outputter);
}

} // weblog
