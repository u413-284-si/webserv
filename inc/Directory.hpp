#pragma once

#include "FileSystemPolicy.hpp"
#include <vector>
#include <algorithm>

/**
 * @brief Wrapper class for DIR pointer.
 *
 * This class is a wrapper for the DIR pointer. It is used to get the entries of a directory.
 * Constructor opens the directory.
 * Destructor closes the directory when the object is destroyed.
 */
class Directory {
public:
	Directory(const FileSystemPolicy& fileSystemPolicy, const std::string& path);
	~Directory();

	std::vector<std::string> getEntries();

private:
	Directory(const Directory&);
	Directory& operator=(const Directory&);

	const FileSystemPolicy& m_fileSystemPolicy;
	DIR* m_directory;
};
