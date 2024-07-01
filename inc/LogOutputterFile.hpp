#pragma once

#include <fstream>
#include <iostream>

#include "ALogOutputter.hpp"

namespace weblog {

class LogOutputterFile : public ALogOutputter {
public:
	explicit LogOutputterFile(const char* filename = "log.txt");

	virtual void log(const LogData& logData);

private:
	std::ofstream m_logfile;
	const char* const m_filename;
	bool m_isOpen;
};

} // weblog
