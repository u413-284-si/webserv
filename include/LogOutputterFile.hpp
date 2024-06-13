#pragma once

#include <fstream>
#include <iostream>

#include "ILogOutputter.hpp"

class LogOutputterFile : public ILogOutputter {
public:
	explicit LogOutputterFile(const char* const filename = "log.txt");

	virtual void log(const LogLevel level, const std::string& message);

private:
	std::ofstream m_logfile;
	const char* const m_filename;
};
