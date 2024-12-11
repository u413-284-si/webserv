#include "FileWriteHandler.hpp"

/**
 * @brief Construct a new FileWriteHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
FileWriteHandler::FileWriteHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Executes the file write operation.
 *
 * This function writes the given content to the specified file path. If the file already exists,
 * the content is appended to the file. If the file does not exist, a new file is created with the
 * given content. The function returns a JSON-formatted body containing details about the operation.
 *
 * @param path The file path where the content should be written.
 * @param content The content to be written to the file.
 * @return A JSON-formatted string containing the operation result, including the file path, file size,
 *         last modified time, and status (either "updated" or "created"). If an error occurs, an empty
 *         string is returned.
 *
 * @exception std::runtime_error If an error occurs during the file operation.
 */
std::string FileWriteHandler::execute(const std::string& path, const std::string& content)
{
	try {
		const bool isExistingFile = m_fileSystemPolicy.isExistingFile(path);
		m_fileSystemPolicy.writeToFile(path, content);
		struct stat fileStat = m_fileSystemPolicy.getFileStat(path);

		if (isExistingFile) {
			m_response << "{\n"
					   << "\"message\": \"Data appended successfully\",\n"
					   << "\"file\": \"" << path << "\",\n"
					   << "\"file_size\": " << m_fileSystemPolicy.getFileSize(fileStat) << ",\n"
					   << "\"last_modified\": \"" << m_fileSystemPolicy.getLastModifiedTime(fileStat) << "\",\n"
					   << "\"status\": \"updated\"\n"
					   << "}\n";
		} else {
			m_response << "{\n"
					   << "\"message\": \"File created successfully\",\n"
					   << "\"file\": \"" << path << "\",\n"
					   << "\"file_size\": " << m_fileSystemPolicy.getFileSize(fileStat) << ",\n"
					   << "\"last_modified\": \"" << m_fileSystemPolicy.getLastModifiedTime(fileStat) << "\",\n"
					   << "\"status\": \"created\"\n"
					   << "}\n";
		}
		return m_response.str();
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		return "";
	}
}
