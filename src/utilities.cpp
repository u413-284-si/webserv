#include "utilities.hpp"
#include <cctype>
#include <sys/socket.h>

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
	std::string::const_iterator iter = str.begin();

	// Find the first character that is not a whitespace
	while (iter != str.end() && (std::isspace(static_cast<unsigned char>(*iter)) != 0))
		++iter;

	// Return the substring starting from the first non-whitespace character
	return std::string(iter, str.end());
}

/**
 * @brief Trims trailing white spaces from a string.
 * 
 * This function removes all trailing white spaces (spaces, tabs, newlines, etc.)
 * from the input string.
 * 
 * @param str The string to be trimmed. The string is modified in place.
 * 
 */
void webutils::trimTrailingWhiteSpaces(std::string& str)
{
	std::string::size_type  end = str.size();

    while (end > 0 && (std::isspace(str.at(end - 1)) != 0))
        --end;
    str.erase(end);
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
	size_t posStart = 0;
	size_t posEnd = 0;
	size_t delimLen = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((posEnd = str.find(delimiter, posStart)) != std::string::npos) {
		token = str.substr(posStart, posEnd - posStart);
		posStart = posEnd + delimLen;
		res.push_back(token);
	}

	res.push_back(str.substr(posStart));
	return res;
}
