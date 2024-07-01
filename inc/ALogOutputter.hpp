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
class ALogOutputter {

public:
	ALogOutputter();
	virtual ~ALogOutputter();

	static std::string getFormattedMessage(const LogData& logData);
	virtual void log(const LogData& logData) = 0;

private:
	ALogOutputter(const ALogOutputter& ref);
	ALogOutputter& operator=(const ALogOutputter& rhs);
};

} // weblog
