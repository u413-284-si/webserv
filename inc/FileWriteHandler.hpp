#pragma once

#include "Directory.hpp"
#include "FileSystemOps.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Class to handle the POST requests.
 *
 */
class FileWriteHandler {

public:
	explicit FileWriteHandler(const FileSystemOps& fileSystemOps);
	std::string execute(const std::string& path, const std::string& content);

private:
	const FileSystemOps& m_fileSystemOps;
	std::stringstream m_response;
};
