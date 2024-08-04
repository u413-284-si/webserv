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
 * @brief PRIVATE copy constructor.
 *
 * Prevts copying.
 *
 * @param ref The reference object to copy.
 */
ALogOutputter::ALogOutputter(const ALogOutputter& ref) { (void)ref; }

/**
 * @brief PRIVATE copy assignment operator.
 *
 * Prevents copying.
 * NOLINT to prevent clang-tidy warning about self-assignment (cert-oop54-cpp).
 *
 * @param rhs The right-hand side object to copy.
 * @return ALogOutputter& The copied object.
 */
ALogOutputter& ALogOutputter::operator=(const ALogOutputter& rhs) //NOLINT
{
	(void)rhs;
	return *this;
}

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
