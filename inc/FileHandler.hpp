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
	static bool isDirectory(const std::string& path);
	static bool isExistingFile(const std::string& path);
	static std::string getFileContents(const char* filename);

private:
	enum fileType { NotExist = 0, Directory = 1, RegularFile = 2, Other = 3 };
	static fileType checkFileType(const std::string& path);
};
