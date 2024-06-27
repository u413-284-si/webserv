#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "Logger.hpp"

#define LOG_INIT(level, outputter) Logger::getInstance().init(level, outputter)
#define LOG_(level) Logger::getInstance() += LogData(level, __PRETTY_FUNCTION__, __LINE__, __FILE__)

#define LOG_DEBUG LOG_(LevelDebug)
#define LOG_INFO LOG_(LevelInfo)
#define LOG_WARN LOG_(LevelWarn)
#define LOG_ERROR LOG_(LevelError)
