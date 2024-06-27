#pragma once

#include "LogData.hpp"

namespace weblog {

/**
 * @brief Interface for log outputters.
 *
 * The ILogOutputter interface is used to implement different log outputters.
 * The log outputters must implement the log function to log messages and the
 * getFormattedMessage to format the message.
 * The copy ctor and copy assignment operator are implemented to silence clang-tidy warnings.
 */
class ILogOutputter {

public:
	ILogOutputter() {};
	virtual ~ILogOutputter() {};

	virtual void log(const LogData& logData) = 0;

private:
	ILogOutputter(const ILogOutputter& ref) { (void)ref; };
	ILogOutputter& operator=(const ILogOutputter& rhs)
	{
		if (this == &rhs)
			return *this;
		(void)rhs;
		return *this;
	};

	virtual std::string getFormattedMessage(const LogData& logData) const = 0;
};

} // weblog
