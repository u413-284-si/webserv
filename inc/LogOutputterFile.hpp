#pragma once

#include <fstream>
#include <iostream>

#include "ILogOutputter.hpp"

class LogOutputterFile : public ILogOutputter {
public:
	explicit LogOutputterFile(const char* filename = "log.txt");

	virtual void log(const LogData& logData);
	virtual std::string getFormattedMessage(const LogData& logData);

private:
	std::ofstream m_logfile;
	const char* const m_filename;
};
