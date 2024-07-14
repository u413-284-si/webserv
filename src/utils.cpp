#include "utils.hpp"

namespace utils {

std::string getFileExtension(const std::string& path)
{
	const std::size_t extPos = path.find_last_of('.');
	std::size_t dirPos = path.find_last_of('/');
	if (dirPos == std::string::npos) {
		dirPos = 0;
	}

	if (extPos != std::string::npos && extPos > dirPos) {
		return path.substr(extPos + 1);
	}
	return "";
}

std::string getGMTString(const char* format)
{
	const time_t now = time(0);

	char string[utils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format, gmtime(&now)));
	return string;
}

std::string getLocaltimeString(const char* format)
{
	const time_t now = time(0);

	char string[utils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format, localtime(&now)));
	return string;
}

} // namespace utils
