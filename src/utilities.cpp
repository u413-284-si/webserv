#include "utilities.hpp"

std::string trimTrailingWhiteSpaces(const std::string &str)
{
    std::string s(str);

    s.erase(
        find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(),
        s.end()
    );
    return s;
}
