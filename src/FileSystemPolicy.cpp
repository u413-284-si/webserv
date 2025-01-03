#include "FileSystemPolicy.hpp"

/**
 * @brief Construct a new File System Policy:: File System Policy object
 *
 */
FileSystemPolicy::FileSystemPolicy() { }

/**
 * @brief Virtual destructor for FileSystemPolicy object
 *
 */
FileSystemPolicy::~FileSystemPolicy() { }

/**
 * @brief Construct a new File System Policy:: File Not Found Exception:: File Not Found Exception object
 *
 * @param msg Error message.
 */
FileSystemPolicy::FileNotFoundException::FileNotFoundException(const std::string& msg)
	: std::runtime_error(msg)
{
}

/**
 * @brief Construct a new File System Policy:: No Permission Exception:: No Permission Exception object
 *
 * @param msg Error message.
 */
FileSystemPolicy::NoPermissionException::NoPermissionException(const std::string& msg)
	: std::runtime_error(msg)
{
}

/**
 * @brief Check the file type of a given path.
 *
 * Uses stat() to check the file type of a given path.
 * If the file is a regular file, FileRegular is returned.
 * If the file is a directory, FileDirectory is returned.
 * If the file is neither a regular file nor a directory, FileOther is returned.
 * If the file does not exist, FileNotFound is returned
 * @throws FileSystemPolicy::NoPermissionException if the file cannot be accessed.
 * @throws std::runtime_error in other cases with strerror() of errno.
 * @param path Path to check.
 * @return FileSystemPolicy::fileType File type.
 */
FileSystemPolicy::fileType FileSystemPolicy::checkFileType(const std::string& path) const
{
	struct stat fileStat = {};
	errno = 0;
	if (stat(path.c_str(), &fileStat) == -1) {
		if (errno == ENOENT)
			return FileNotFound;
		if (errno == EACCES)
			throw NoPermissionException("stat(): " + std::string(strerror(errno)));
		throw std::runtime_error("stat(): " + std::string(strerror(errno)));
	}
	// NOLINTNEXTLINE: misinterpretation by HIC++ standard
	if (S_ISREG(fileStat.st_mode))
		return FileRegular;
	// NOLINTNEXTLINE: misinterpretation by HIC++ standard
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
bool FileSystemPolicy::isExistingFile(const std::string& path) const { return checkFileType(path) != FileNotFound; }

/**
 * @brief Gets the contents of a file.
 *
 * Opens a file stream. Checks if it could open wirh is_open(). If not throws custom exceptions.
 * If the stream has other errors, throws a std::runtime_error.
 * @throws FileSystemPolicy::FileNotFoundException if the file does not exist.
 * @throws FileSystemPolicy::NoPermissionException if the file cannot be accessed.
 * @throws std::runtime_error in other cases with strerror() of errno.
 * @param filename File to read.
 * @return std::string File contents.
 */
std::string FileSystemPolicy::getFileContents(const char* filename) const
{
	errno = 0;
	std::ifstream fileStream(filename, std::ios::in | std::ios::binary);
	if (!fileStream.is_open()) {
		if (errno == ENOENT)
			throw FileNotFoundException("std::ifstream: " + std::string(strerror(errno)));
		if (errno == EACCES)
			throw NoPermissionException("std::ifstream: " + std::string(strerror(errno)));
	}
	if (fileStream.fail())
		throw std::runtime_error("std::ifstream: " + std::string(strerror(errno)));
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
	DIR* dir = opendir(path.c_str());
	if (dir == NULL)
		throw std::runtime_error("opendir(): " + std::string(strerror(errno)));
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
	struct dirent* entry = readdir(dir);
	if (entry == NULL && errno != 0)
		throw std::runtime_error("readdir(): " + std::string(strerror(errno)));
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
	if (closedir(dir) == -1)
		return (errno);
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
	if (stat(path.c_str(), &fileStat) == -1)
		throw std::runtime_error("stat(): " + std::string(strerror(errno)));
	return fileStat;
}

/**
 * @brief Creates a file with the specified content at the given path.
 *
 * This function attempts to create a file at the specified path and writes the provided content to it.
 * If the file cannot be created, an error is logged and the function returns false.
 * If the file exists already, the content is appended to the file.
 *
 * @throws FileSystemPolicy::FileNotFoundException if the file does not exist.
 * @throws FileSystemPolicy::NoPermissionException if the file cannot be accessed.
 * @throws std::runtime_error in other cases with strerror() of errno.
 * @param path The path where the file should be created.
 * @param content The content to be written to the file.
 */
void FileSystemPolicy::writeToFile(const std::string& path, const std::string& content) const
{
	errno = 0;
	std::ofstream file(path.c_str(), std::ios::binary | std::ios::app);
	if (!file.is_open()) {
		if (errno == ENOENT)
			throw FileNotFoundException("std::ifstream: " + std::string(strerror(errno)));
		if (errno == EACCES)
			throw NoPermissionException("std::ifstream: " + std::string(strerror(errno)));
	}
	if (file.fail())
		throw std::runtime_error("std::ifstream: " + std::string(strerror(errno)));
	file << content;
}

/**
 * @brief Get the last modified time of a file.
 *
 * @param fileStat File stat object.
 * @return std::string Last modified time.
 */
std::string FileSystemPolicy::getLastModifiedTime(const struct stat& fileStat) const
{
	return webutils::getLocaltimeString(fileStat.st_mtime, "%Y-%m-%d %H:%M:%S");
}

/**
 * @brief Get the file size of a file.
 *
 * @param fileStat File stat object.
 * @return long File size.
 */
long FileSystemPolicy::getFileSize(const struct stat& fileStat) const
{
	// NOLINTNEXTLINE: misinterpretation by HIC++ standard
	if (S_ISDIR(fileStat.st_mode))
		return 0;
	return fileStat.st_size;
}

/**
 * @brief Deletes a file at the specified path.
 *
 * This function attempts to delete the file located at the given path.
 * If the file cannot be deleted, it throws a NoPermissionException if the errno
 * code is 13 (= EACCES), else it throws a runtime error with the
 * appropriate error message.
 *
 * @param path The path to the file to be deleted.
 * @throws std::runtime_error if the file cannot be deleted.
 */
void FileSystemPolicy::deleteFile(const std::string& path) const
{
	errno = 0;
	if (std::remove(path.c_str()) != 0) {
		if (errno == EACCES)
			throw NoPermissionException("remove(): " + std::string(strerror(errno)));
		throw std::runtime_error("remove(): " + std::string(strerror(errno)));
	}
}
