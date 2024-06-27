#include "LogOutputterFile.hpp"

namespace weblog {

/**
 * @brief Construct a new Log Outputter File:: Log Outputter File object
 *
 * The constructor initializes the filename and the open flag.
 * @param filename The name of the logfile.
 */
LogOutputterFile::LogOutputterFile(const char* filename)
	: m_filename(filename)
	, m_isOpen(false)
{
}

/**
 * @brief Log a message to the file.
 *
 * The log function logs the message to the file.
 * If the bool m_isOpen is false, the file is opened.
 * If the file could not be opened, an error message is printed to std::cerr.
 * The message is formatted with the getFormattedMessage function.
 * After every log message, the file is flushed.
 * @param logData The log data to log.
 */
void LogOutputterFile::log(const LogData& logData)
{
	if (!m_isOpen) {
		m_logfile.open(m_filename, std::ios::out | std::ios::app);
		if (!m_logfile.is_open()) {
			std::cerr << "error: could not open file " << m_filename << '\n';
			return;
		}
		m_isOpen = true;
	}
	const std::string message = getFormattedMessage(logData);
	m_logfile << message;
	m_logfile.flush();
}

/**
 * @brief Get the formatted message.
 *
 * The getFormattedMessage function formats the log message.
 * The message is formatted with
 * - the time,
 * - the loglevel,
 * - (if LevelDebug: the function, the file and the line number)
 * - the message.
 * @param logData The log data to format.
 * @return std::string The formatted message.
 */
std::string LogOutputterFile::getFormattedMessage(const LogData& logData) const
{
	std::stringstream message;

	message << logData.getFormattedTime();

	message << " [" << LogData::levelToString(logData.getLevel()) << "] ";

	if (logData.getLevel() == LevelDebug) {
		message << "<" << logData.getFunction() << ">(" << logData.getFile() << ":" << logData.getLine() << "): ";
	}

	message << logData.getMessage() << '\n';

	return message.str();
}

} // weblog
