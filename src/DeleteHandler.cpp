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


std::string DeleteHandler::execute(const std::string& path)
{
	try {

		return m_response.str();
	} catch (std::runtime_error& e) {
		LOG_ERROR << e.what();
		return "";
	}
}
