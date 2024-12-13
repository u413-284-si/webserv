#pragma once

#include "Directory.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Class to handle the POST requests.
 *
 */
class DeleteHandler {

public:
	explicit DeleteHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
