#include "AutoindexHandler.hpp"
#include "Directory.hpp"

AutoindexHandler::AutoindexHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

std::string getLastModifiedTime(const struct stat& fileStat)
{
	char timeStr[100];
	(void)strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));
	return std::string(timeStr);
}

long getFileSize(const struct stat& fileStat)
{
	if (S_ISDIR(fileStat.st_mode))
		return 0;
	return fileStat.st_size;
}

std::string AutoindexHandler::execute(const std::string& path)
{
	try {
		m_response
			<< "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
			<< "<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
			<< "<title>Autoindex</title>\n"
			<< "<style>\n"
			<< "table { width: 100%; border-collapse: collapse; }\n"
			<< "table, th, td { border: 1px solid black; }\n"
			<< "th, td { padding: 10px; text-align: left; }\n"
			<< "th { background-color: #f2f2f2; }\n"
			<< "</style>\n</head>\n<body>\n"
			<< "<h1>Index of " << path << "</h1>\n"
			<< "<table>\n"
			<< "<tr><th>File Name</th><th>Last Modified</th><th>Size (Bytes)</th></tr>\n";

		Directory directory(m_fileSystemPolicy, path);
		std::vector<std::string> files = directory.getEntries();

		for (std::vector<std::string>::iterator iter = files.begin(); iter != files.end(); ++iter) {
			if (*iter == "." || *iter == "..")
				continue;
			struct stat fileStat = m_fileSystemPolicy.getFileStat(path + *iter);
			if (S_ISDIR(fileStat.st_mode))
				*iter += "/";
			m_response << "<tr><td><a href=\"" << *iter << "\">" << *iter << "</a></td>"
					   << "<td>" << getLastModifiedTime(fileStat) << "</td>"
					   << "<td>" << getFileSize(fileStat) << "</td></tr>\n";
		}
		m_response << "</table>\n</body>\n</html>";
		return m_response.str();
	} catch (std::runtime_error& e) {
		std::cerr << "error: " << e.what() << "\n";
		return "";
	}
}
