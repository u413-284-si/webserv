#pragma once

#include "Directory.hpp"
#include "FileSystemOps.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Class to create or append to files.
 *
 */
class FileWriteHandler {

public:
	explicit FileWriteHandler(const FileSystemOps& fileSystemOps);
	std::string execute(const std::string& path, const std::string& content, statusCode& httpStatus);

private:
	const FileSystemOps& m_fileSystemOps;
	std::stringstream m_response;
};
