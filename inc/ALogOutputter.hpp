#pragma once

#include "LogData.hpp"

namespace weblog {

/**
 * @brief Abstract base class for log outputters.
 *
 * The ALogOutputter base class is used to implement different log outputters.
 * The log outputters must implement the log function to log messages to its respective output.
 * The copy ctor and copy assignment operator are private to prevent copying of the object.
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
