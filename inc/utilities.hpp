#pragma once

/* ====== LIBRARIES ====== */

#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>

/* ====== DECLARATIONS ====== */

std::string					trimTrailingWhiteSpaces(const std::string &str);
std::vector<std::string>	split(const std::string &str, char delimiter);
