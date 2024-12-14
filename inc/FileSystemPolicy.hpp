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
#include <unistd.h>

#include "utilities.hpp"
#include "Log.hpp"

/**
 * @brief Class for C functions on the file system.
 *
 * This class is a wrapper for C functions on the file system.
 * It is used to make the code more testable.
 * It contains a nested DirectoryGuard class for managing directory pointers.
 * The class ensures that open pointers are closed when they go out of scope.
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
	virtual std::string getLastModifiedTime(const struct stat& fileStat) const;
	virtual long getFileSize(const struct stat& fileStat) const;
	virtual void deleteFile(const std::string& path) const;
	virtual void deleteDirectory(const std::string& path) const;

private:
    class DirectoryGuard {
    public:
        explicit DirectoryGuard(DIR* dir);
        ~DirectoryGuard();
        DIR* getDir() const;

    private:
        DIR* m_dir;
    };
};
