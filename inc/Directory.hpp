#pragma once

#include "FileSystemPolicy.hpp"
#include <vector>
#include <algorithm>

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
