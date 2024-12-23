#pragma once

#include "LogOutputterConsole.hpp"
#include "LogOutputterFile.hpp"
#include "Logger.hpp"

#define LOG_INIT(level, outputter) weblog::Logger::getInstance().init(level, outputter)
#define LOG_(level) weblog::Logger::getInstance() += weblog::LogData(level, __PRETTY_FUNCTION__, __LINE__, __FILE__)

#ifdef DEBUG_MSG
 #define LOG_DEBUG LOG_(weblog::LevelDebug)
#else
 #define LOG_DEBUG if (0) std::cerr
#endif

#define LOG_INFO LOG_(weblog::LevelInfo)
#define LOG_WARN LOG_(weblog::LevelWarn)
#define LOG_ERROR LOG_(weblog::LevelError)

namespace weblog {

Logger& initFile(LogLevel level, const char* filename);
Logger& initConsole(LogLevel level);

} // weblog
