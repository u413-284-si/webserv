#pragma once

#include <cerrno>
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
 * @brief Class for C functions on the file system.
 *
 * This class is a wrapper for C functions on the file system.
 * It is used to make the code more testable.
 */
class FileSystemPolicy {

public:
	FileSystemPolicy();
	virtual ~FileSystemPolicy();

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
	virtual bool isRegularFile(const std::string& path) const;
	virtual std::string getFileContents(const char* filename) const;
	virtual DIR* openDirectory(const std::string& path) const;
	virtual struct dirent* readDirectory(DIR* dir) const;
	virtual int closeDirectory(DIR* dir) const;
	virtual struct stat getFileStat(const std::string& path) const;
	virtual void writeToFile(const std::string& path, const std::string& content) const;
	virtual std::string getLastModifiedTime(const struct stat& fileStat) const;
	virtual long getFileSize(const struct stat& fileStat) const;

private:
	FileSystemPolicy(const FileSystemPolicy& ref);
	FileSystemPolicy& operator=(const FileSystemPolicy& ref);
};
