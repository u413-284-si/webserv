#include "utilities.hpp"

/**
 * @brief Removes leading whitespaces from a given string.
 *
 * This function iterates over the input string to find the first
 * non-whitespace character and returns a substring starting from
 * that character to the end of the original string.
 *
 * @param str The input string from which to remove leading whitespaces.
 * @return A new string with leading whitespaces removed.
 */
std::string webutils::trimLeadingWhitespaces(const std::string& str)
{
	std::string::const_iterator it = str.begin();

	// Find the first character that is not a whitespace
	while (it != str.end() && std::isspace(static_cast<unsigned char>(*it)))
		++it;

	// Return the substring starting from the first non-whitespace character
	return std::string(it, str.end());
}

/**
 * @brief Removes trailing whitespaces from a given string.
 *
 * This function creates a copy of the input string and removes any
 * trailing whitespace characters from it. The modified string is
 * then returned.
 *
 * @param str The input string from which to remove trailing whitespaces.
 * @return A new string with trailing whitespaces removed.
 */
std::string webutils::trimTrailingWhiteSpaces(const std::string& str)
{
	std::string s(str);

	s.erase(find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

/**
 * @brief Splits a given string into substrings based on a specified delimiter.
 *
 * This function splits the input string `str` into a vector of substrings,
 * using the specified `delimiter` to determine where the splits occur.
 * Each substring is added to the result vector, which is returned at the end.
 *
 * @param str The input string to be split.
 * @param delimiter The string delimiter used to split the input string.
 * @return A vector of substrings obtained by splitting the input string based on the delimiter.
 */
std::vector<std::string> webutils::split(const std::string& str, const std::string& delimiter)
{
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
		token = str.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(str.substr(pos_start));
	return res;
}
