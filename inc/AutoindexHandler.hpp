#pragma once

#include "Directory.hpp"
#include "FileSystemOps.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Class to handle the autoindex feature.
 *
 */
class AutoindexHandler {

public:
	explicit AutoindexHandler(const FileSystemOps& fileSystemOps);
	std::string execute(const std::string& path, const std::string& uriPath);

private:
	const FileSystemOps& m_fileSystemOps;
	std::stringstream m_response;
};
