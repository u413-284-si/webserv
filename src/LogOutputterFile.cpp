#include "LogOutputterFile.hpp"

LogOutputterFile::LogOutputterFile(const char* const filename)
	: m_filename(filename)
{
	m_logfile.open(m_filename, std::ios_base::app);
	if (!m_logfile.is_open())
		std::cerr << "Error: Could not open file: " << m_filename << '\n';
}

void LogOutputterFile::log(const LogData& logData)
{
	const std::string message = getFormattedMessage(logData);
	m_logfile << message;
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
