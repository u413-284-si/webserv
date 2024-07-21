#pragma once

#include <iostream>
#include <map>

#include "ALogOutputter.hpp"

namespace weblog {

class LogOutputterConsole : public ALogOutputter {

public:
	LogOutputterConsole();
	virtual void log(const LogData& logData);

private:
	enum TermColors { None = 0, Green = 1, Yellow = 2, Red = 3, Count = 4 };

	void initColorMap();

	std::map<TermColors, const char*> m_colors;
};

} // weblog
