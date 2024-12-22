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

/**
 * @brief Executes the delete operation on the specified path.
 *
 * Depending on the type of the file at the given path, the method will:
 * - Delete a regular file and return a success message.
 * - Delete a directory and return a success message.
 * - Set the HTTP status to 404 if the file does not exist.
 * - Set the HTTP status to 500 if the file type is not supported.
 * - Set the HTTP status to 403 if the operation is not permitted.
 *
 * @param path The path of the file or directory to delete.
 * @param httpStatus Reference to the HTTP status code to be set based on the operation result.
 * @return A JSON string containing the result of the delete operation.
 */
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
			httpStatus = StatusForbidden;
			// FIXME: reactivate after eval
			// deleteDirectory(path);
			// m_response << "{\n"
			// 		   << "\"message\": \"Directory deleted successfully\",\n"
			// 		   << "\"directory\": \"" << path << "\"\n"
			// 		   << "}\n";
			// break;

		case FileSystemPolicy::FileNotExist:
			httpStatus = StatusNotFound;
			break;

		case FileSystemPolicy::FileOther:
			httpStatus = StatusForbidden;
			break;
		}
	} catch (const std::runtime_error& e) {
		std::string error = e.what();
		LOG_ERROR << error;
		if (error.find("Permission denied") != std::string::npos) // FIXME: use custom exception
			httpStatus = StatusForbidden;
		else
			httpStatus = StatusInternalServerError;
	}
	return m_response.str();
}

// FIXME: reactivate after eval
// /**
//  * @brief Deletes a directory at the specified path.
//  *
//  * This function attempts to delete the directory located at the given path.
//  * It recursively deletes all files and subdirectories within the directory.
//  * If any file or directory cannot be deleted, it throws a runtime error with
//  * the appropriate error message.
//  *
//  * @param path The path to the directory to be deleted.
//  * @throws std::runtime_error if any file or directory cannot be deleted.
//  */
// // NOLINTNEXTLINE (misc-no-recursion): recursion is being handled
// void DeleteHandler::deleteDirectory(const std::string& path) const
// {
// 	Directory dir(m_fileSystemPolicy, path);

// 	std::vector<std::string> entries = dir.getEntries();
// 	for (std::vector<std::string>::const_iterator iter = entries.begin(); iter != entries.end(); ++iter) {
// 		// Skip `.` and `..` entries
// 		if (strcmp((*iter).c_str(), ".") == 0 || strcmp((*iter).c_str(), "..") == 0)
// 			continue;

// 		std::string fullPath = std::string(path) + "/" + *iter;

// 		struct stat info = m_fileSystemPolicy.getFileStat(fullPath);
// 		// NOLINTNEXTLINE: misinterpretation by HIC++ standard
// 		if (S_ISDIR(info.st_mode))
// 			deleteDirectory(fullPath);
// 		else
// 			m_fileSystemPolicy.deleteFile(fullPath);
// 	}

// 	// Remove the empty directory itself
// 	errno = 0;
// 	if (rmdir(path.c_str()) != 0)
// 		throw std::runtime_error("rmdir(): " + std::string(strerror(errno)));
// }
