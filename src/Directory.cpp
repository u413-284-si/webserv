#include "Directory.hpp"

/**
 * @brief Construct a new Directory object
 *
 * @param fileSystemOps Wrapper for filesystem-related functions. Can be mocked if needed.
 * @param path Path to directory.
 */
Directory::Directory(const FileSystemOps& fileSystemOps, const std::string& path)
	: m_fileSystemOps(fileSystemOps)
	, m_directory(fileSystemOps.openDirectory(path))
{
}

/**
 * @brief Destroy Directory object
 *
 * Closes the directory.
 */
Directory::~Directory() { m_fileSystemOps.closeDirectory(m_directory); }

/**
 * @brief Get the entries in the directory.
 *
 * Entries are sorted alphabetically.
 * @return std::vector<std::string> List of entries in the directory.
 */
std::vector<std::string> Directory::getEntries()
{
	std::vector<std::string> files;
	struct dirent* entry = m_fileSystemOps.readDirectory(m_directory);
	while (entry != NULL) {
		files.push_back(entry->d_name);
		entry = m_fileSystemOps.readDirectory(m_directory);
	}
	std::sort(files.begin(), files.end());
	return files;
}
