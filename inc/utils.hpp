#pragma once

#include <string>
#include <ctime>

namespace utils {

const int timeStringBuffer = 80;

std::string getFileExtension(const std::string& path);
std::string getGMTString(const char* format);
std::string getLocaltimeString(const char* format);

} // namespace utils
