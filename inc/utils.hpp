#pragma once

#include <string>
#include <sys/stat.h>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace utils {
	enum fileType { NotExist = 0, Directory = 1, RegularFile = 2, Other = 3 };
	fileType checkFileType(const std::string& path);
} // namespace utils
