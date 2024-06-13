#pragma once

#include <string>

#include "LogLevel.hpp"

class ILogOutputter {
public:
	virtual ~ILogOutputter() {};

	virtual void log(const LogLevel level, const std::string& message) = 0;
};
