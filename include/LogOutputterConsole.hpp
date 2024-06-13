#pragma once

#include <iostream>

#include "ILogOutputter.hpp"

class LogOutputterConsole : public ILogOutputter {
public:
	virtual void log(const LogLevel level, const std::string& message);
};
