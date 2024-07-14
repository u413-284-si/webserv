#include "FileSystemPolicy.hpp"

/**
 * @brief Virtual destructor for FileSystemPolicy object
 *
 */
FileSystemPolicy::~FileSystemPolicy() {}

/**
 * @brief Check the file type of a given path.
 *
 * Uses stat() to check the file type of a given path.
 * If the file does not exist, FileNotExist is returned.
 * If the file is a regular file, FileRegular is returned.
 * If the file is a directory, FileDirectory is returned.
 * If the file is neither a regular file nor a directory, FileOther is returned.
 * @throws std::runtime_error with strerror() of errno.
 * @param path Path to check.
 * @return FileSystemPolicy::fileType File type.
 */
FileSystemPolicy::fileType FileSystemPolicy::checkFileType(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT)
			return FileNotExist;
		throw std::runtime_error(strerror(errno));
	}
	if (S_ISREG(fileStat.st_mode))
		return FileRegular;
	if (S_ISDIR(fileStat.st_mode))
		return FileDirectory;
	return FileOther;
}

/**
 * @brief Wrapper to check if a path is a directory.
 *
 * @param path Path to check.
 * @return true Path is a directory.
 * @return false Path is not a directory.
 */
bool FileSystemPolicy::isDirectory(const std::string& path) const { return checkFileType(path) == FileDirectory; }

/**
 * @brief Wrapper to check if a path is an existing file.
 *
 * @param path Path to check.
 * @return true Path is an existing file.
 * @return false Path is not an existing file.
 */
bool FileSystemPolicy::isExistingFile(const std::string& path) const { return checkFileType(path) != FileNotExist; }

/**
 * @brief Gets the contents of a file.
 *
 * @throws std::runtime_error with strerror() of errno.
 * @param filename File to read.
 * @return std::string File contents.
 */
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

/**
 * @brief Opens a directory.
 *
 * @throws std::runtime_error with strerror() of errno.
 * @param path Path to open.
 * @return DIR* Directory stream.
 */
DIR* FileSystemPolicy::openDirectory(const std::string& path) const
{
	errno = 0;
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		throw std::runtime_error(strerror(errno));
	}
	return dir;
}

/**
 * @brief Reads a directory.
 *
 * @throws std::runtime_error with strerror() of errno.
 * @param dir Directory stream.
 * @return struct dirent* Directory entry.
 */
struct dirent* FileSystemPolicy::readDirectory(DIR* dir) const
{
	errno = 0;
	struct dirent *entry = readdir(dir);
	if (entry == NULL && errno != 0) {
		throw std::runtime_error(strerror(errno));
	}
	return entry;
}

/**
 * @brief Closes a directory.
 *
 * @param dir Directory stream.
 * @return int errno Error code.
 */
int FileSystemPolicy::closeDirectory(DIR* dir) const
{
	errno = 0;
	if (closedir(dir) == -1) {
		return (errno);
	}
	return 0;
}

/**
 * @brief Gets the file status of a path.
 *
 * @throws std::runtime_error with strerror() of errno.
 * @param path Path to check.
 * @return struct stat File status.
 */
struct stat FileSystemPolicy::getFileStat(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		throw std::runtime_error(strerror(errno));
	}
	return fileStat;
}
