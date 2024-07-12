#pragma once

#include "FileSystemPolicy.hpp"
#include "Directory.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

class AutoindexHandler {

public:
	explicit AutoindexHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path);

private:
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};
