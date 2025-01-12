#pragma once

#include "FileSystemOps.hpp"
#include <algorithm>
#include <vector>

/**
 * @brief Wrapper class for DIR pointer.
 *
 * This class is a wrapper for the DIR pointer. It is used to get the entries of a directory.
 * Constructor opens the directory.
 * Destructor closes the directory when the object is destroyed.
 */
class Directory {
public:
	Directory(const FileSystemOps& fileSystemOps, const std::string& path);
	~Directory();

	std::vector<std::string> getEntries();

private:
	Directory(const Directory& ref);
	Directory& operator=(const Directory& ref);

	const FileSystemOps& m_fileSystemOps;
	DIR* m_directory;
};
