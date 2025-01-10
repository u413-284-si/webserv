#pragma once

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "Log.hpp"
#include "utilities.hpp"

/**
 * @brief Wrapper class for filesystem-related functions.
 *
 * This class provides wrappers for c-functions interacting with the filesystem. The following functions are wrapped:
 * - stat()
 * - opendir()
 * - readdir()
 * - closedir()
 * It also provides a wrapper for filestreams to get the content of a file and write to a file.
 * It can also be mocked for testing purposes.
 */
class FileSystemOps {

public:
	FileSystemOps();
	virtual ~FileSystemOps();

	enum fileType { FileNotFound = 0, FileDirectory = 1, FileRegular = 2, FileOther = 3 };

	// custom exceptions
	struct FileNotFoundException : public std::runtime_error {
		explicit FileNotFoundException(const std::string& msg);
	};
	struct NoPermissionException : public std::runtime_error {
		explicit NoPermissionException(const std::string& msg);
	};

	virtual fileType checkFileType(const std::string& path) const;
	virtual bool isDirectory(const std::string& path) const;
	virtual bool isExistingFile(const std::string& path) const;
	virtual std::string getFileContents(const char* filename) const;
	virtual DIR* openDirectory(const std::string& path) const;
	virtual struct dirent* readDirectory(DIR* dir) const;
	virtual int closeDirectory(DIR* dir) const;
	virtual struct stat getFileStat(const std::string& path) const;
	virtual void writeToFile(const std::string& path, const std::string& content) const;
	virtual std::string getLastModifiedTime(const struct stat& fileStat) const;
	virtual long getFileSize(const struct stat& fileStat) const;
	virtual void deleteFile(const std::string& path) const;

private:
	FileSystemOps(const FileSystemOps& ref);
	FileSystemOps& operator=(const FileSystemOps& ref);
};
