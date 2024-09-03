#include "ALogOutputter.hpp"

namespace weblog {

/**
 * @brief Construct a new ALogOutputter::ALogOutputter object
 *
 * Default constructor for inheritance.
 *
 */
ALogOutputter::ALogOutputter() { }

/**
 * @brief Destroy the ALogOutputter::ALogOutputter object
 *
 * Virtual destrcutor.
 *
 */
ALogOutputter::~ALogOutputter() { }

/**
 * @brief Format a message out of the logData object.
 *
 * The message is formatted with
 * - the time,
 * - the loglevel,
 * - (if LevelDebug: the function, the file and the line number)
 * - the message.
 *
 * @param logData The log data to format.
 * @return std::string The formatted message.
 */
std::string ALogOutputter::getFormattedMessage(const LogData& logData)
{
	std::stringstream message;

	message << logData.getFormattedTime();
	message << " [" << LogData::levelToString(logData.getLevel()) << "] ";
	if (logData.getLevel() == LevelDebug)
		message << "<" << logData.getFunction() << ">(" << logData.getFile() << ":" << logData.getLine() << "): ";
	message << logData.getMessage();
	return message.str();
}

} // weblog
