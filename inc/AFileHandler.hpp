#pragma once

#include "FileSystemPolicy.hpp"
#include "Directory.hpp"
#include "Log.hpp"
#include "utilities.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

/**
 * @brief Abstract class to provide file handling features.
 *
 */
class AFileHandler {

public:
	explicit AFileHandler(const FileSystemPolicy& fileSystemPolicy);
	virtual ~AFileHandler();

	virtual std::string execute(const std::string& path) = 0;
	virtual std::string execute(const std::string& path, const std::string& content) = 0;

	// Getters
	std::stringstream& getResponse();
    const FileSystemPolicy& getFileSystemPolicy() const;

	static std::string getLastModifiedTime(const struct stat& fileStat);
	static long getFileSize(const struct stat& fileStat);

private:
	AFileHandler(const AFileHandler& ref);
	AFileHandler& operator=(const AFileHandler& rhs);
	
	const FileSystemPolicy& m_fileSystemPolicy;
	std::stringstream m_response;
};

