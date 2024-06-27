#pragma once

#include <string>
#include <vector>

#include "ILogOutputter.hpp"
#include "LogData.hpp"

class Logger {
public:
	static Logger& getInstance();
	Logger& addLogOutputter(ILogOutputter* outputter);

	void operator+=(const LogData& logData);

	static Logger& init(LogLevel level = LevelInfo, ILogOutputter* outputter = NULL);
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
	std::vector<ILogOutputter*> m_outputters;
};
