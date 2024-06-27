#pragma once

#include <iostream>
#include <map>

#include "ILogOutputter.hpp"

class LogOutputterConsole : public ILogOutputter {
public:
	LogOutputterConsole();
	virtual void log(const LogData& logData);

private:
	virtual std::string getFormattedMessage(const LogData& logData);
	enum TermColors { NONE = 0, RED = 1, YELLOW = 2, GREEN = 3, COUNT = 4 };
	void initColorMap();

	std::map<TermColors, const char *> m_colors;
};
