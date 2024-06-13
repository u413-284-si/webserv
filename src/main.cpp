#include "Log.hpp"

int main()
{
	static LogOutputterConsole console;
	static LogOutputterFile file("test.txt");
	Logger::getInstance().addLogOutputter(&console);
	Logger::getInstance().addLogOutputter(&file);
	LOG_DEBUG << "This is a debug message";
	LOG_INFO << "This is a Info message";
	LOG_WARN << "This is warn message";
	LOG_ERROR << "This is error message";
}
