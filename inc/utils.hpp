#pragma once

#include <string>
#include <ctime>
#include "StatusCode.hpp"

namespace webutils {

const int timeStringBuffer = 80;

std::string getFileExtension(const std::string& path);
std::string getGMTString(time_t now, const char* format);
std::string getLocaltimeString(time_t now, const char* format);

std::string statusCodeToReasonPhrase(statusCode status);
std::string getDefaultErrorPage(statusCode status);

} // namespace utils
