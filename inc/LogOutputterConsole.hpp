#pragma once

#include <iostream>
#include <map>

#include "ILogOutputter.hpp"

namespace weblog {

class LogOutputterConsole : public ILogOutputter {

public:
	LogOutputterConsole();
	virtual void log(const LogData& logData);

private:
	virtual std::string getFormattedMessage(const LogData& logData) const;

	enum TermColors { None = 0, Red = 1, Yellow = 2, Green = 3, Count = 4 };

	void initColorMap();

	std::map<TermColors, const char*> m_colors;
};

} // weblog
