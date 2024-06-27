#include "Log.hpp"

namespace weblog {

/**
 * @brief Initialize the Logger object with a log level and a file outputter.
 *
 * The LogOutputterFile is static object, it is created the first time this function is called (lazy initialization).
 * It then calls the init function of the Logger object.
 * Since it is a static object, it is created only once and the same object is returned every time this function is
 * called.
 * @param level The log level.
 * @param filename The name of the logfile.
 * @return Logger& The Logger object.
 */
Logger& initFile(LogLevel level, const char* filename)
{
	static LogOutputterFile outputter(filename);
	return Logger::init(level, &outputter);
}

/**
 * @brief Initialize the Logger object with a log level and a console outputter.
 *
 * The LogOutputterConsole is static object, it is created the first time this function is called (lazy initialization).
 * It then calls the init function of the Logger object.
 * Since it is a static object, it is created only once and the same object is returned every time this function is
 * called.
 * @param level The log level.
 * @return Logger& The Logger object.
 */
Logger& initConsole(LogLevel level)
{
	static LogOutputterConsole outputter;
	return Logger::init(level, &outputter);
}

} // weblog
