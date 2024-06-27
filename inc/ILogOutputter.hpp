#pragma once

#include "LogData.hpp"

class ILogOutputter {
public:
	ILogOutputter() {};
	virtual ~ILogOutputter() {};

	virtual void log(const LogData& logData) = 0;
	
private:
	ILogOutputter(const ILogOutputter& ref) { (void)ref; };
	ILogOutputter& operator=(const ILogOutputter& rhs) { if (this == &rhs) return *this; (void)rhs; return *this; };

	virtual std::string getFormattedMessage(const LogData& logData) = 0;
};
