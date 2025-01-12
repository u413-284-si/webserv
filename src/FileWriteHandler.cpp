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
 * @param httpStatus Reference to the HTTP status code to be set based on the operation result.
 * @return A JSON-formatted string containing the operation result, including the file path, file size,
 *         last modified time, and status (either "updated" or "created"). If an error occurs, an empty
 *         string is returned.
 *
 * @exception std::runtime_error If an error occurs during the file operation.
 */
std::string FileWriteHandler::execute(const std::string& path, const std::string& content, statusCode& httpStatus)
{
	try {
		const bool isExistingFile = m_fileSystemPolicy.isExistingFile(path);
		m_fileSystemPolicy.writeToFile(path, content);
		LOG_DEBUG << "Data successfully written/appended to the file: " << path;
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
			httpStatus = StatusCreated;
			m_response << "{\n"
					   << "\"message\": \"File created successfully\",\n"
					   << "\"file\": \"" << path << "\",\n"
					   << "\"file_size\": " << m_fileSystemPolicy.getFileSize(fileStat) << ",\n"
					   << "\"last_modified\": \"" << m_fileSystemPolicy.getLastModifiedTime(fileStat) << "\",\n"
					   << "\"status\": \"created\"\n"
					   << "}\n";
		}
	} catch (FileSystemPolicy::NoPermissionException& e) {
		LOG_ERROR << e.what();
		httpStatus = StatusForbidden;
	} catch (FileSystemPolicy::FileNotFoundException& e) {
		LOG_ERROR << e.what();
		LOG_ERROR << "Missing directory in path " << path;
		httpStatus = StatusNotFound;
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		LOG_ERROR << "Failed to write data to the file: " << path;
		httpStatus = StatusInternalServerError;
	}
	return m_response.str();
}
