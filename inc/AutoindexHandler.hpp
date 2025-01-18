#pragma once

#include "FileSystemPolicy.hpp"
#include "Directory.hpp"
#include "utilities.hpp"
#include "Log.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

/**
 * @brief Class to handle the autoindex feature.
 *
 */
class AutoindexHandler {

public:
	explicit AutoindexHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path, const std::string& uriPath);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
