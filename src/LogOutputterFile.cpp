#include "LogOutputterFile.hpp"

LogOutputterFile::LogOutputterFile(const char* filename)
	: m_filename(filename), m_isOpen(false)
{
}

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

std::string LogOutputterFile::getFormattedMessage(const LogData& logData)
{
	std::stringstream message;

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

	message << logData.getMessage() << '\n';

	return message.str();
}
