#pragma once

#include <string>
#include <vector>

#include "ALogOutputter.hpp"
#include "LogData.hpp"

namespace weblog {

/**
 * @brief Logger class to log messages.
 *
 * Implements the Singleton pattern to log messages to different outputters.
 * The Logger class is a Singleton class, so it can be accessed from anywhere in the code.
 * The Logger class can log messages to different outputters, which implement the ILogOutputter interface.
 * The Logger class can log messages with different log levels, which are defined in the LogLevel enum.
 * The Logger class can be initialized with a log level and an outputter.
 *
 */
class Logger {

public:
	static Logger& getInstance();
	Logger& addLogOutputter(ALogOutputter* outputter);

	void operator+=(const LogData& logData) const;

	static Logger& init(LogLevel level = LevelInfo, ALogOutputter* outputter = NULL);
	LogLevel getLevel() const;
	Logger& setLevel(LogLevel level);

private:
	// private constructor to prevent direct object creation
	Logger();

	// disallow copying the instance
	Logger(const Logger& ref);
	Logger& operator=(const Logger& ref);
	~Logger() {};

	LogLevel m_logLevel;
	std::vector<ALogOutputter*> m_outputters;
};

} // weblog
