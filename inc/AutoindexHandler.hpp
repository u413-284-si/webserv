#pragma once

#include "AFileHandler.hpp"

/**
 * @brief Class to handle the autoindex feature.
 *
 */
class AutoindexHandler : public AFileHandler {

public:
	explicit AutoindexHandler(const FileSystemPolicy& fileSystemPolicy);

	virtual std::string execute(const std::string& path);

private:
	virtual std::string execute(const std::string& path, const std::string& content);
};
