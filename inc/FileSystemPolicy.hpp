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

class FileSystemPolicy {

public:
	virtual ~FileSystemPolicy();

	enum fileType { StatError = -1, FileNotExist = 0, FileDirectory = 1, FileRegular = 2, FileOther = 3 };
	virtual fileType checkFileType(const std::string& path) const;
	virtual bool isDirectory(const std::string& path) const;
	virtual bool isExistingFile(const std::string& path) const;
	virtual std::string getFileContents(const char* filename) const;
	virtual DIR* openDirectory(const std::string& path) const;
	virtual struct dirent* readDirectory(DIR* dir) const;
	virtual struct stat getFileStat(const std::string& path) const;
};
