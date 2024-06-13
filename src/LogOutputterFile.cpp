#include "LogOutputterFile.hpp"

LogOutputterFile::LogOutputterFile(const char* const filename)
	: m_filename(filename)
{
	m_logfile.open(m_filename, std::ios_base::app);
	if (!m_logfile.is_open())
		std::cerr << "Error: Could not open file: " << m_filename << '\n';
}

void LogOutputterFile::log(const LogLevel level, const std::string& message)
{
	(void)level;
	m_logfile << message;
}
