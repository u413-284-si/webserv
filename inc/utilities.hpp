#pragma once

/* ====== LIBRARIES ====== */

#include "constants.hpp"
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

bool isIpAddressValid(const std::string& ipAddress);
bool isPortValid(const std::string& port);

void lowercase(std::string& str);

/**
 * @brief Convert a type to a string
 *
 * @tparam Type The type to convert
 * @param type The type to convert
 * @return std::string The string representation of the type
 * @todo Delete if not needed anymore
 */
template <typename Type> std::string toString(const Type& type)
{
	std::stringstream stream;
	stream << type;
	return stream.str();
}

} // webutils
