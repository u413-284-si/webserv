#pragma once

/* ====== LIBRARIES ====== */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

/* ====== DECLARATIONS ====== */

namespace webutils {

std::string trimLeadingWhitespaces(const std::string& str);
void trimTrailingWhiteSpaces(std::string& str);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);

} // webutils
