#include "utilities.hpp"

std::string webutils::trimLeadingWhitespaces(const std::string& str)
{
	std::string::const_iterator it = str.begin();

	// Find the first character that is not a whitespace
	while (it != str.end() && std::isspace(static_cast<unsigned char>(*it)))
		++it;

	// Return the substring starting from the first non-whitespace character
	return std::string(it, str.end());
}

std::string webutils::trimTrailingWhiteSpaces(const std::string& str)
{
	std::string s(str);

	s.erase(find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

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
