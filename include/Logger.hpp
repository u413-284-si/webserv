#pragma once

#include <string>
#include <vector>

#include "ILogOutputter.hpp"
#include "LogData.hpp"
#include "LogLevel.hpp"

class Logger {
public:
	static Logger& getInstance();
	Logger& addLogOutputter(ILogOutputter* outputter);

	void operator+=(const LogData& record);

	LogLevel getLevel() const;
	Logger& setLevel(LogLevel level);

private:
	// private constructor to prevent direct object creation
	Logger();

	// disallow copying the instance
	Logger(const Logger& ref);
	Logger& operator=(const Logger& ref);

	static std::string getFormattedMessage(const LogData& record);

	LogLevel m_logLevel;
	std::vector<ILogOutputter*> m_outputters;
};
