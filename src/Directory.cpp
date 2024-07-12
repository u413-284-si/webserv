#include "Directory.hpp"

Directory::Directory(const FileSystemPolicy& fileSystemPolicy, const std::string& path)
	: m_fileSystemPolicy(fileSystemPolicy)
	, m_directory(fileSystemPolicy.openDirectory(path))
{
}

Directory::Directory(const Directory& ref) : m_fileSystemPolicy(ref.m_fileSystemPolicy), m_directory(ref.m_directory)
{
}

Directory& Directory::operator=(const Directory& ref)
{
	if (this == &ref)
		return *this;
	m_directory = ref.m_directory;
	return *this;
}

Directory::~Directory()
{
	m_fileSystemPolicy.closeDirectory(m_directory);
}

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
