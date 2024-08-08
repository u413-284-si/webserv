#pragma once

/* ====== LIBRARIES ====== */

#include "StatusCode.hpp"
#include <algorithm>
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

/**
 * @brief Convert a type to a string
 *
 * @tparam Type
 * @param type
 * @return std::string
 * @todo Delete if not needed anymore
 */
template <typename Type> std::string toString(const Type& type)
{
	std::stringstream stream;
	stream << type;
	return stream.str();
}

} // webutils
