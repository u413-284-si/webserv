#include "AutoindexHandler.hpp"

/**
 * @brief Construct a new AutoindexHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
AutoindexHandler::AutoindexHandler(const FileSystemPolicy& fileSystemPolicy)
	: AFileHandler(fileSystemPolicy)
{
}

/**
 * @brief Execute the autoindex handler.
 *
 * Generates an HTML response with the contents of a directory.
 * The response contains a table with the file names, last modified time and file size.
 * The file names are links to the files.
 * If a function throws, returns empty string.
 * @param path Path to directory.
 * @return std::string HTML response.
 */
std::string AutoindexHandler::execute(const std::string& path)
{
	try {
		getResponse()
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

		Directory directory(getFileSystemPolicy(), path);
		std::vector<std::string> files = directory.getEntries();

		for (std::vector<std::string>::iterator iter = files.begin(); iter != files.end(); ++iter) {
			if (*iter == "." || *iter == "..")
				continue;
			struct stat fileStat = getFileSystemPolicy().getFileStat(path + *iter);
			if (S_ISDIR(fileStat.st_mode))
				*iter += "/";
			getResponse() << "<tr><td><a href=\"" << *iter << "\">" << *iter << "</a></td>"
						  << "<td>" << getLastModifiedTime(fileStat) << "</td>"
						  << "<td>" << getFileSize(fileStat) << "</td></tr>\n";
		}
		getResponse() << "</table>\n</body>\n</html>";
		return getResponse().str();
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		return "";
	}
}

/**
 * @brief Placeholder function required by the base class.
 * 
 * This function cannot be called and is not implemented in the AutoindexHandler class.
 * 
 * @param path Path to file.
 * @param content Content to be written to the file.
 * @return An empty string.
 */
std::string AutoindexHandler::execute(const std::string& path, const std::string& content)
{
	(void)path;
	(void)content;
	return "";
}
