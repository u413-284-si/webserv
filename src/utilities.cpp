#include "utilities.hpp"

std::string trimLeadingWhitespaces(const std::string& str) {
    std::string::const_iterator it = str.begin();

    // Find the first character that is not a whitespace
    while (it != str.end() && std::isspace(static_cast<unsigned char>(*it)))
        ++it;

    // Return the substring starting from the first non-whitespace character
    return std::string(it, str.end());
}

std::string trimTrailingWhiteSpaces(const std::string &str)
{
    std::string s(str);

    s.erase(
        find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(),
        s.end()
    );
    return s;
}

std::vector<std::string>	split(const std::string &str, char delimiter)
{
    std::vector<std::string>	tokens;
    std::stringstream			ss(str);
    std::string					token;

    while (std::getline(ss >> std::ws, token, delimiter)) {
		token = trimTrailingWhiteSpaces(token);
        tokens.push_back(token);
    }
    return tokens;
}
