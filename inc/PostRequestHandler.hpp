#pragma once

#include "AFileHandler.hpp"
/**
 * @brief Class to handle the POST requests.
 *
 */
class PostRequestHandler : public AFileHandler {

public:
	explicit PostRequestHandler(const FileSystemPolicy& fileSystemPolicy);
	std::string execute(const std::string& path, const std::string& content);

private:
	std::string execute(const std::string& path);
};
