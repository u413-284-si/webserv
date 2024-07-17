#pragma once

/* ====== LIBRARIES ====== */

#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>

/* ====== DECLARATIONS ====== */

namespace webutils {

std::string trimLeadingWhitespaces(const std::string& str);
std::string					trimTrailingWhiteSpaces(const std::string &str);
std::vector<std::string>	split(const std::string &str, const std::string &delimiter);

} // webutils
