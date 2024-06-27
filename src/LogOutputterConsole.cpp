#include "LogOutputterConsole.hpp"

namespace weblog
{

LogOutputterConsole::LogOutputterConsole()
{
	initColorMap();
}

void LogOutputterConsole::log(const LogData& logData)
{
	const std::string message = getFormattedMessage(logData);
	if (logData.getLevel() >= LevelWarn)
		std::cerr << message;
	else
		std::clog << message;
}

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

	switch (logData.getLevel()) {
	case LevelDebug:
		message << "[DEBUG] ";
		break;
	case LevelInfo:
		message << "[INFO] ";
		break;
	case LevelWarn:
		message << "[WARN] ";
		break;
	case LevelError:
		message << "[ERROR] ";
		break;
	}

	if (logData.getLevel() == LevelDebug) {
		message << "<" << logData.getFunction() << ">(" << logData.getFile() << ":" << logData.getLine() << "): ";
	}

	message << logData.getMessage() << m_colors.at(NONE) << '\n';

	return message.str();
}

void LogOutputterConsole::initColorMap()
{
	m_colors[NONE] = "\033[0m";
	m_colors[RED] = "\033[31m";
	m_colors[YELLOW] = "\033[33m";
	m_colors[GREEN] = "\033[32m";
}

} // weblog
