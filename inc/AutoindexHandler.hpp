#pragma once

#include "FileHandler.hpp"
#include <string>
#include <vector>
#include <sstream>

class AutoindexHandler {

public:
	explicit AutoindexHandler(const FileHandler& fileHandler);
	std::string execute(const std::string& path);

private:
	std::vector<std::string> getFiles(DIR* directory);

	const FileHandler& m_fileHandler;
	std::stringstream m_response;
};
