#include "Logger.hpp"

Logger::Logger()
	: m_logLevel(LevelDebug)
{
}

Logger& Logger::addLogOutputter(ILogOutputter* outputter)
{
	m_outputters.push_back(outputter);
	return *this;
}

Logger::Logger(const Logger& ref)
	: m_logLevel(ref.getLevel())
{
	(void)ref;
}

Logger& Logger::operator=(const Logger& ref)
{
	if (this == &ref) {
		return *this;
	}
	m_logLevel = ref.getLevel();
	return *this;
}

Logger& Logger::getInstance()
{
	static Logger instance;
	return instance;
}

LogLevel Logger::getLevel() const { return m_logLevel; }

Logger& Logger::setLevel(LogLevel level)
{
	m_logLevel = level;
	return *this;
}

void Logger::operator+=(const LogData& record)
{
	std::string message = getFormattedMessage(record);

	if (record.getLevel() >= getLevel()) {
		for (std::vector<ILogOutputter*>::iterator it = m_outputters.begin(); it != m_outputters.end(); ++it) {
			(*it)->log(getLevel(), message);
		}
	}
}

std::string Logger::getFormattedMessage(const LogData& record)
{
	// Create a string stream for formatting
	std::stringstream message;

	// Add timestamp
	message << record.getFormattedTime();

	// Add log level prefix
	switch (record.getLevel()) {
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

	// Append function name, file name and line number if LogLevel is Debug
	if (record.getLevel() == LevelDebug) {
		message << "<" << record.getFunction() << ">(" << record.getFile() << ":" << record.getLine() << "): ";
	}

	// Append the actual message
	message << record.getMessage();

	// Append a newline
	message << '\n';

	return message.str();
}
