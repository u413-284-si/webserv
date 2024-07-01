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
	m_logfile << getFormattedMessage(logData) << '\n';
	m_logfile.flush();
}

} // weblog
