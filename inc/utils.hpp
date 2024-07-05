#pragma once

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <stdexcept>

namespace utils {

enum fileType { NotExist = 0, Directory = 1, RegularFile = 2, Other = 3 };
fileType checkFileType(const std::string& path);
bool isDirectory(const std::string& path);
bool isExistingFile(const std::string& path);
std::string getFileContents(const char* filename);

} // namespace utils
