#include "LogOutputterConsole.hpp"

namespace weblog {

/**
 * @brief Construct a new Log Outputter Console:: Log Outputter Console object
 *
 * The constructor initializes the color map.
 */
LogOutputterConsole::LogOutputterConsole() { initColorMap(); }

/**
 * @brief Initialize the color map.
 *
 * The color map is used to color the log messages.
 * The color map is a map of LogLevel and the corresponding color.
 */
void LogOutputterConsole::initColorMap()
{
	m_colors[NONE] = "\033[0m";
	m_colors[RED] = "\033[31m";
	m_colors[YELLOW] = "\033[33m";
	m_colors[GREEN] = "\033[32m";
}

/**
 * @brief Log a message to the console.
 *
 * The log function logs the message to the console.
 * The message is formatted with the getFormattedMessage function.
 * The message is logged to std::clog for LevelDebug and LevelInfo messages.
 * The message is logged to std::cerr for LevelWarn and LevelError messages.
 * @param logData The log data to log.
 */
void LogOutputterConsole::log(const LogData& logData)
{
	const std::string message = getFormattedMessage(logData);
	if (logData.getLevel() >= LevelWarn)
		std::cerr << message;
	else
		std::clog << message;
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
 * The message is colored
 * - yellow for LevelWarn messages.
 * - red for LevelError messages.
 * @param logData The log data to format.
 * @return std::string The formatted message.
 */
std::string LogOutputterConsole::getFormattedMessage(const LogData& logData) const
{
	std::stringstream message;

	switch (logData.getLevel()) {
	case LevelWarn:
		message << m_colors.at(YELLOW);
		break;
	case LevelError:
		message << m_colors.at(RED);
		break;
	default:
		break;
	}

	message << logData.getFormattedTime();

	message << " [" << LogData::levelToString(logData.getLevel()) << "] ";

	if (logData.getLevel() == LevelDebug) {
		message << "<" << logData.getFunction() << ">(" << logData.getFile() << ":" << logData.getLine() << "): ";
	}

	message << logData.getMessage() << m_colors.at(NONE) << '\n';

	return message.str();
}

} // weblog
