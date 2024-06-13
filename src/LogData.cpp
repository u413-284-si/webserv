#include "LogData.hpp"

LogData::LogData(LogLevel level, const char* function, size_t line, const char* file)
	: m_level(level)
	, m_function(function)
	, m_line(line)
	, m_file(file)
{
	formatTime();
}

const std::string& LogData::getMessage() const { return m_message; }

const LogLevel& LogData::getLevel() const { return m_level; }

const std::string& LogData::getFormattedTime() const { return m_formattedTime; }

const size_t& LogData::getLine() const { return m_line; }

const char* LogData::getFunction() const { return m_function; }

const char* LogData::getFile() const { return m_file; }

void LogData::formatTime()
{
	// Get current time
	time_t now = time(0);

	// Use ctime to convert time_t to a human-readable string (not thread-safe)
	const int maxTimeString = 80;
	char buffer[maxTimeString];
	(void)strftime((char*)buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", localtime(&now));
	m_formattedTime = (char*)buffer;
}
