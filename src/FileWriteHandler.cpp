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
