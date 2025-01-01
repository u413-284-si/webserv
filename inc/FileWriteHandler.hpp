#pragma once

#include "Directory.hpp"
#include "FileSystemPolicy.hpp"
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
	explicit FileWriteHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path, const std::string& content);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
