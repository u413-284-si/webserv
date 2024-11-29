#include "AFileHandler.hpp"

/**
 * @brief Construct a new AFileHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
AFileHandler::AFileHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Destroy the AFileHandler::AFileHandler object
 *
 */
AFileHandler::~AFileHandler() { }

/**
 * @brief Retrieves the response stream.
 *
 * This method provides access to the response stream, which can be used
 * to build and retrieve the response content.
 *
 * @return A reference to the response stringstream.
 */
std::stringstream& AFileHandler::getResponse() { return m_response; }

/**
 * @brief Retrieves the file system policy.
 *
 * This method provides access to the file system policy associated with
 * the file handler. The file system policy defines the rules and behaviors
 * for file system operations.
 *
 * @return A constant reference to the FileSystemPolicy object.
 */
const FileSystemPolicy& AFileHandler::getFileSystemPolicy() const { return m_fileSystemPolicy; }

/**
 * @brief Get the last modified time of a file.
 *
 * @param fileStat File stat object.
 * @return std::string Last modified time.
 */
std::string AFileHandler::getLastModifiedTime(const struct stat& fileStat)
{
	return webutils::getLocaltimeString(fileStat.st_mtime, "%Y-%m-%d %H:%M:%S");
}

/**
 * @brief Get the file size of a file.
 *
 * @param fileStat File stat object.
 * @return long File size.
 */
long AFileHandler::getFileSize(const struct stat& fileStat)
{
	if (S_ISDIR(fileStat.st_mode))
		return 0;
	return fileStat.st_size;
}

