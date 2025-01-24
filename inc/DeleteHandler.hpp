#pragma once

#include "Directory.hpp"
#include "FileSystemOps.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Class to delete files and directories.
 *
 */
class DeleteHandler {

public:
	explicit DeleteHandler(const FileSystemOps& fileSystemOps);
	std::string execute(const std::string& path, statusCode& httpStatus);

private:
	const FileSystemOps& m_fileSystemOps;
	std::stringstream m_response;

	// FIXME: activate after eval
	// void deleteDirectory(const std::string& path) const;
};
