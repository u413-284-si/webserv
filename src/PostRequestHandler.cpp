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
		if (getFileSystemPolicy().isExistingFile(path))
        {
            // Attempt to append data to file
			// Create response according to result of attempt
        }

        if (getFileSystemPolicy().isFileCreated(path, content)) {
			getResponse()
				<< "{\n"
				<< "\"file\":" << path << ",\n"
				<< "\"file_size\":" << getFileSize(getFileSystemPolicy().getFileStat(path)) << ",\n"
				<< "\"status\": \"created\"\n"
				<< "}\n";
		} else
			return "";
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
