#include "LogData.hpp"

namespace weblog {

LogData::LogData(LogLevel level, const char* function, size_t line, const char* file)
	: m_level(level)
	, m_function(function)
	, m_line(line)
	, m_file(file)
{
	formatTime();
}

const std::string& LogData::getMessage() const { return m_message; }

const LogLevel& LogData::getLevel() const { return m_level; }

const std::string& LogData::getFormattedTime() const { return m_formattedTime; }

const size_t& LogData::getLine() const { return m_line; }

const char* LogData::getFunction() const { return m_function; }

const char* LogData::getFile() const { return m_file; }

void LogData::formatTime()
{
	// Get current time
	time_t now = time(0);

	// Use ctime to convert time_t to a human-readable string (not thread-safe)
	const int maxTimeString = 80;
	char buffer[maxTimeString];
	(void)strftime((char*)buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", localtime(&now));
	m_formattedTime = (char*)buffer;
}

LogData& LogData::operator<<(const ConfigFile& configFile)
{
	for (std::vector<ServerConfig>::const_iterator it = configFile.serverConfigs.begin();
		 it != configFile.serverConfigs.end(); ++it) {
		m_stream << "Server: " << it->serverName << '\n';
		m_stream << "Host: " << it->host << '\n';
		m_stream << "Port: " << it->port << '\n';
		m_stream << "Max body size: " << it->maxBodySize << '\n';
		m_stream << "Error pages:\n";
		for (std::map<unsigned short, std::string>::const_iterator it2 = it->errorPage.begin();
			 it2 != it->errorPage.end(); ++it2) {
			m_stream << "  " << it2->first << ": " << it2->second << '\n';
		}
		m_stream << "Locations:\n";
		for (std::vector<Location>::const_iterator it2 = it->locations.begin(); it2 != it->locations.end(); ++it2) {
			m_stream << "  Path: " << it2->path << '\n';
			m_stream << "  Root: " << it2->root << '\n';
			m_stream << "  Index: " << it2->index << '\n';
			m_stream << "  CGI extension: " << it2->cgiExt << '\n';
			m_stream << "  CGI path: " << it2->cgiPath << '\n';
			m_stream << "  Autoindex: " << it2->isAutoindex << '\n';
			m_stream << "  LimitExcept:\n";
			m_stream << "    Allow: " << it2->limitExcept.allow << '\n';
			m_stream << "    Deny: " << it2->limitExcept.deny << '\n';
			m_stream << "    Allowed methods:\n";
			for (int i = 0; i < MethodCount; ++i) {
				m_stream << "      " << i << ": " << it2->limitExcept.allowedMethods[i] << '\n';
			}
			m_stream << "  Returns:\n";
			for (std::map<unsigned short, std::string>::const_iterator it3 = it2->returns.begin();
				 it3 != it2->returns.end(); ++it3) {
				m_stream << "    " << it3->first << ": " << it3->second << '\n';
			}
		}
		m_stream << '\n';
	}
	m_message = m_stream.str();
	return *this;
}

} // weblog
