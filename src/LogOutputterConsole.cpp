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
	m_colors[None] = "\033[0m";
	m_colors[Green] = "\033[32m";
	m_colors[Yellow] = "\033[33m";
	m_colors[Red] = "\033[31m";
}

/**
 * @brief Log a message to the console.
 *
 * The log function logs the message to the console.
 * The message is formatted with the getFormattedMessage function.
 * The message is logged to std::clog for LevelDebug and LevelInfo messages.
 * The message is logged to std::cerr for LevelWarn (yellow) and LevelError (red) messages.
 * @param logData The log data to log.
 */
void LogOutputterConsole::log(const LogData& logData)
{
	switch (logData.getLevel()) {
	case LevelWarn:
		std::cerr << m_colors.at(Yellow) << getFormattedMessage(logData) << m_colors.at(None) << '\n';
		break;
	case LevelError:
		std::cerr << m_colors.at(Red) << getFormattedMessage(logData) << m_colors.at(None) << '\n';
		break;
	default:
		std::clog << getFormattedMessage(logData) << '\n';
	}
}

} // weblog
