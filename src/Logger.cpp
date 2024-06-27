#include "Logger.hpp"

namespace weblog
{

Logger::Logger() : m_logLevel(LevelInfo)
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

Logger& Logger::init(LogLevel level, ILogOutputter* outputter)
{
	Logger& logger = Logger::getInstance();
	logger.setLevel(level);
	if (outputter != NULL) {
		logger.addLogOutputter(outputter);
	}
	return logger;
}

LogLevel Logger::getLevel() const { return m_logLevel; }

Logger& Logger::setLevel(LogLevel level)
{
	m_logLevel = level;
	return *this;
}

void Logger::operator+=(const LogData& logData)
{
	if (logData.getLevel() >= getLevel()) {
		for (std::vector<ILogOutputter*>::iterator it = m_outputters.begin(); it != m_outputters.end(); ++it) {
			(*it)->log(logData);
		}
	}
}

} // weblog
