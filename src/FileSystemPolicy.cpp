#include "FileSystemPolicy.hpp"

FileSystemPolicy::~FileSystemPolicy() {}

FileSystemPolicy::fileType FileSystemPolicy::checkFileType(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT)
			return FileNotExist;
		std::cerr << "error: stat: " << strerror(errno) << "\n";
		return StatError;
	}
	if (S_ISREG(fileStat.st_mode))
		return FileRegular;
	if (S_ISDIR(fileStat.st_mode))
		return FileDirectory;
	return FileOther;
}

bool FileSystemPolicy::isDirectory(const std::string& path) const { return checkFileType(path) == FileDirectory; }

bool FileSystemPolicy::isExistingFile(const std::string& path) const { return checkFileType(path) != FileNotExist; }

std::string FileSystemPolicy::getFileContents(const char* filename) const
{
	errno = 0;
	std::ifstream fileStream(filename, std::ios::in | std::ios::binary);
	if (!fileStream.good()) {
		throw std::runtime_error(strerror(errno));
	}
	std::string contents;
	fileStream.seekg(0, std::ios::end);
	contents.resize(fileStream.tellg());
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(&contents[0], static_cast<std::streamsize>(contents.size()));
	return contents;
}

DIR* FileSystemPolicy::openDirectory(const std::string& path) const
{
	errno = 0;
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		throw std::runtime_error(strerror(errno));
	}
	return dir;
}

struct dirent* FileSystemPolicy::readDirectory(DIR* dir) const
{
	errno = 0;
	struct dirent *entry = readdir(dir);
	if (entry == NULL && errno != 0) {
		throw std::runtime_error(strerror(errno));
	}
	return entry;
}

void FileSystemPolicy::closeDirectory(DIR* dir) const
{
	errno = 0;
	if (closedir(dir) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}

struct stat FileSystemPolicy::getFileStat(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		throw std::runtime_error(strerror(errno));
	}
	return fileStat;
}
