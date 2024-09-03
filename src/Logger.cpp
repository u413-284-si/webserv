#include "Logger.hpp"

namespace weblog {

/**
 * @brief PRIVATE Construct a new Logger:: Logger object.

 * The constructor is private to prevent direct object creation.
 * The LogLevel is set to silence clang-tidy warning.
 */
Logger::Logger()
	: m_logLevel(LevelInfo)
{
}

/**
 * @brief PRIVATE Destroy the Logger:: Logger object
 *
 * The destructor is private to prevent direct object deletion.
 */
Logger::~Logger() {}

/**
 * @brief Get the Logger object instance.
 *
 * The Logger object is a Singleton, so it can be accessed from anywhere in the code.
 * The Logger object is created the first time this function is called (lazy initialization).
 * @return Logger& The Logger object.
 */
Logger& Logger::getInstance()
{
	static Logger instance;
	return instance;
}

/**
 * @brief Add a log outputter to the Logger object.
 *
 * The Logger object can log messages to different outputters, which implement the ILogOutputter interface.
 * @param outputter The ILogOutputter object to add.
 * @return Logger& The Logger object.
 */
Logger& Logger::addLogOutputter(ALogOutputter* outputter)
{
	m_outputters.push_back(outputter);
	return *this;
}

/**
 * @brief Initialize the Logger object with a log level and an outputter.
 *
 * @param level The LogLevel to set.
 * @param outputter The ILogOutputter object to add.
 * @return Logger& The Logger object.
 */
Logger& Logger::init(LogLevel level, ALogOutputter* outputter)
{
	Logger& logger = Logger::getInstance();
	logger.setLevel(level);
	if (outputter != NULL)
		logger.addLogOutputter(outputter);
	return logger;
}

/**
 * @brief Get the log level of the Logger object.
 *
 * @return LogLevel The log level.
 */
LogLevel Logger::getLevel() const { return m_logLevel; }

/**
 * @brief Set the log level of the Logger object.
 *
 * @param level The LogLevel to set.
 * @return Logger& The Logger object.
 */
Logger& Logger::setLevel(LogLevel level)
{
	m_logLevel = level;
	return *this;
}

/**
 * @brief Log a message with the given log data.
 *
 * This overload passes the LogData object to all registered the log outputters.
 * The log message is only logged if the log level of the log data is greater or equal to the log level of the Logger
 * object.
 * @param logData The LogData object to log.
 */
void Logger::operator+=(const LogData& logData) const
{
	if (logData.getLevel() >= getLevel()) {
		for (std::vector<ALogOutputter*>::const_iterator it = m_outputters.begin(); it != m_outputters.end(); ++it)
			(*it)->log(logData);
	}
}

} // weblog
