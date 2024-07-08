#pragma once

#include <string>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <fstream>
#include <stdexcept>
#include <iostream>

class FileHandler {

public:
	bool isDirectory(const std::string& path) const;
	bool isExistingFile(const std::string& path) const;
	std::string getFileContents(const char* filename) const;

private:
	enum fileType { NotExist = 0, Directory = 1, RegularFile = 2, Other = 3 };
	fileType checkFileType(const std::string& path) const;
};
