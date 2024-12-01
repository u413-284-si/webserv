#pragma once

#include <string>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>

/**
 * @brief Class for C functions on the file system.
 *
 * This class is a wrapper for C functions on the file system.
 * It is used to make the code more testable.
 */
class FileSystemPolicy {

public:
	virtual ~FileSystemPolicy();

	enum fileType { FileNotExist = 0, FileDirectory = 1, FileRegular = 2, FileOther = 3 };
	virtual fileType checkFileType(const std::string& path) const;
	virtual bool isDirectory(const std::string& path) const;
	virtual bool isExistingFile(const std::string& path) const;
	virtual std::string getFileContents(const char* filename) const;
	virtual DIR* openDirectory(const std::string& path) const;
	virtual struct dirent* readDirectory(DIR* dir) const;
	virtual int closeDirectory(DIR* dir) const;
	virtual struct stat getFileStat(const std::string& path) const;
	virtual void writeToFile(const std::string& path, const std::string& content) const;
};
