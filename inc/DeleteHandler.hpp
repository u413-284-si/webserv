#pragma once

#include "Directory.hpp"
#include "FileSystemPolicy.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * @brief Class to handle the DELETE requests.
 *
 */
class DeleteHandler {

public:
	explicit DeleteHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path, statusCode& httpStatus);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
