#pragma once

#include "FileSystemPolicy.hpp"
#include "Directory.hpp"
#include "utilities.hpp"
#include "Log.hpp"

#include <string>
#include <sstream>
#include <stdexcept>

/**
 * @brief Class to handle the POST requests.
 *
 */
class PostRequestHandler{

public:
	explicit PostRequestHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path, const std::string& content);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
