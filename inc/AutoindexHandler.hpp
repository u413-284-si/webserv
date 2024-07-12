#pragma once

#include "FileSystemPolicy.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include <stdexcept>

class AutoindexHandler {

public:
	explicit AutoindexHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path);

private:
	std::vector<std::string> getFiles(DIR* directory);

	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
