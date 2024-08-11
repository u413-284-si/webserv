#pragma once

/* ====== LIBRARIES ====== */

#include "ConfigFile.hpp"
#include "StatusCode.hpp"
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>

/* ====== DECLARATIONS ====== */

namespace webutils {

const int timeStringBuffer = 80;

std::string trimLeadingWhitespaces(const std::string& str);
void trimTrailingWhiteSpaces(std::string& str);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);

std::string getFileExtension(const std::string& path);
std::string getGMTString(time_t now, const std::string& format);
std::string getLocaltimeString(time_t now, const std::string& format);

std::string statusCodeToReasonPhrase(statusCode status);
std::string getDefaultErrorPage(statusCode status);

std::string methodToString(Method method);

} // webutils
