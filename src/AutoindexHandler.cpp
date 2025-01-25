#include "AutoindexHandler.hpp"

/**
 * @brief Construct a new AutoindexHandler object
 *
 * @param fileSystemOps Wrapper for filesystem-related functions. Can be mocked if needed.
 */
AutoindexHandler::AutoindexHandler(const FileSystemOps& fileSystemOps)
	: m_fileSystemOps(fileSystemOps)
{
}

/**
 * @brief Execute the autoindex handler.
 *
 * Generates an HTML response with the contents of a directory.
 * The response contains a table with the file names, last modified time and file size.
 * The file names are relative links to the files constructed with the uriPath parameter.
 * If a function throws, returns empty string.
 * @param path Path to directory.
 * @param uriPath URI path which lead to the directory.
 * @return std::string HTML response.
 */
std::string AutoindexHandler::execute(const std::string& path, const std::string& uriPath)
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

		Directory directory(m_fileSystemOps, path);
		std::vector<std::string> files = directory.getEntries();

		for (std::vector<std::string>::iterator iter = files.begin(); iter != files.end(); ++iter) {
			if (*iter == "." || *iter == "..")
				continue;
			struct stat fileStat = m_fileSystemOps.getFileStat(path + *iter);
			// NOLINTNEXTLINE: misinterpretation by HIC++ standard
			if (S_ISDIR(fileStat.st_mode))
				*iter += "/";
			m_response << "<tr><td><a href=\"" << uriPath << *iter << "\">" << *iter << "</a></td>"
					   << "<td>" << m_fileSystemOps.getLastModifiedTime(fileStat) << "</td>"
					   << "<td>" << m_fileSystemOps.getFileSize(fileStat) << "</td></tr>\n";
		}
		m_response << "</table>\n</body>\n</html>";
		return m_response.str();
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		return "";
	}
}
