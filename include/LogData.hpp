#pragma once

#include <ctime>
#include <sstream>
#include <string>

#include "LogLevel.hpp"

class LogData {
public:
	LogData(LogLevel level, const char* function, size_t line, const char* file);

	const std::string& getMessage() const;
	const LogLevel& getLevel() const;
	const std::string& getFormattedTime() const;
	const size_t& getLine() const;
	const char* getFunction() const;
	const char* getFile() const;

	template <typename T> LogData& operator<<(const T& value)
	{
		m_stream << value;
		m_message = m_stream.str();
		return *this;
	}

private:
	// Function to format the timestamp (C++98)
	void formatTime();

	std::ostringstream m_stream;

	std::string m_formattedTime;
	std::string m_message;
	LogLevel m_level;
	const char* m_function;
	size_t m_line;
	const char* m_file;
};
