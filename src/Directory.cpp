#include "Directory.hpp"

/**
 * @brief Construct a new Directory object
 *
 * @param fileSystemPolicy File system policy object. Can be mocked if needed.
 * @param path Path to directory.
 */
Directory::Directory(const FileSystemPolicy& fileSystemPolicy, const std::string& path)
	: m_fileSystemPolicy(fileSystemPolicy)
	, m_directory(fileSystemPolicy.openDirectory(path))
{
}

/**
 * @brief Copy construct a new Directory object
 *
 * Is private, should not be used.
 * @param ref Directory object to copy.
 */
Directory::Directory(const Directory& ref) : m_fileSystemPolicy(ref.m_fileSystemPolicy), m_directory(ref.m_directory)
{
}

/**
 * @brief Copy assign a Directory object
 *
 * Is private, should not be used.
 * @param ref Directory object to copy.
 * @return Directory& Reference to this object.
 */
Directory& Directory::operator=(const Directory& ref)
{
	if (this == &ref)
		return *this;
	m_directory = ref.m_directory;
	return *this;
}

/**
 * @brief Destroy Directory object
 *
 * Closes the directory.
 */
Directory::~Directory()
{
	m_fileSystemPolicy.closeDirectory(m_directory);
}

/**
 * @brief Get the entries in the directory.
 *
 * Entries are sorted alphabetically.
 * @return std::vector<std::string> List of entries in the directory.
 */
std::vector<std::string> Directory::getEntries()
{
	std::vector<std::string> files;
	struct dirent* entry = m_fileSystemPolicy.readDirectory(m_directory);
	while (entry != NULL) {
		files.push_back(entry->d_name);
		entry = m_fileSystemPolicy.readDirectory(m_directory);
	}
	std::sort(files.begin(), files.end());
	return files;
}
