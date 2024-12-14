#include "DeleteHandler.hpp"

/**
 * @brief Construct a new DeleteHandler object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 */
DeleteHandler::DeleteHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}


std::string DeleteHandler::execute(const std::string& path, statusCode& httpStatus)
{
	try {
		FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(path);

		switch (fileType) {

		case FileSystemPolicy::FileRegular:
			m_fileSystemPolicy.deleteFile(path);
			m_response << "{\n"
					   << "\"message\": \"File deleted successfully\",\n"
					   << "\"file\": \"" << path << "\"\n"
					   << "}\n";
			break;

		case FileSystemPolicy::FileDirectory:
			m_fileSystemPolicy.deleteDirectory(path);
			m_response << "{\n"
					   << "\"message\": \"Directory deleted successfully\",\n"
					   << "\"directory\": \"" << path << "\"\n"
					   << "}\n";
			break;

		case FileSystemPolicy::FileNotExist:
			httpStatus = StatusNotFound;
			break;

		case FileSystemPolicy::FileOther:
			httpStatus = StatusInternalServerError;
			break;
		}
	} catch (const std::runtime_error& e) {
		LOG_ERROR << e.what();
		// FIXME: return body depending on error status 403 or 409
	}
	return m_response.str();
}
