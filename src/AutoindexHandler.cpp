#include "AutoindexHandler.hpp"
#include "utils.hpp"

/**
 * @brief Construct a new AutoindexHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
AutoindexHandler::AutoindexHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Get the last modified time of a file.
 *
 * @param fileStat File stat object.
 * @return std::string Last modified time.
 */
std::string getLastModifiedTime(const struct stat& fileStat)
{
	return webutils::getLocaltimeString(fileStat.st_mtime, "%Y-%m-%d %H:%M:%S");
}

/**
 * @brief Get the file size of a file.
 *
 * @param fileStat File stat object.
 * @return long File size.
 */
long getFileSize(const struct stat& fileStat)
{
	if (S_ISDIR(fileStat.st_mode))
		return 0;
	return fileStat.st_size;
}

/**
 * @brief Execute the autoindex handler.
 *
 * Generates an HTML response with the contents of a directory.
 * The response contains a table with the file names, last modified time and file size.
 * The file names are links to the files.
 * If a function throws, retunes an empty string.
 * @param path Path to directory.
 * @return std::string HTML response.
 */
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
