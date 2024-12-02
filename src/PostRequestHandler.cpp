#include "PostRequestHandler.hpp"


/**
 * @brief Construct a new PostRequestHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
PostRequestHandler::PostRequestHandler(const FileSystemPolicy& fileSystemPolicy)
	: AFileHandler(fileSystemPolicy)
{
}

std::string PostRequestHandler::execute(const std::string& path, const std::string& content)
{
	try {
		bool isExistingFile = getFileSystemPolicy().isExistingFile(path);
		getFileSystemPolicy().writeToFile(path, content);
		struct stat fileStat = getFileSystemPolicy().getFileStat(path);
		
		if (isExistingFile)
		{
			getResponse()
				<< "{\n"
				<< "\"message\": \"Data appended successfully\",\n"
				<< "\"file\": \"" << path << "\",\n"
				<< "\"file_size\": " << getFileSize(fileStat) << ",\n"
				<< "\"last_modified\": \"" << getLastModifiedTime(fileStat) << "\",\n"
				<< "\"status\": \"updated\"\n"
				<< "}\n";
		} else {
			getResponse()
				<< "{\n"
				<< "\"message\": \"File created successfully\",\n"
				<< "\"file\": \"" << path << "\",\n"
				<< "\"file_size\": " << getFileSize(fileStat) << ",\n"
				<< "\"last_modified\": \"" << getLastModifiedTime(fileStat) << "\",\n"
				<< "\"status\": \"created\"\n"
				<< "}\n";
		}
		return getResponse().str();
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		return "";
	}
}

std::string PostRequestHandler::execute(const std::string& path)
{
	(void)path;
	return "";
}
