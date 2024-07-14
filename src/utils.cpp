#include "utils.hpp"

namespace utils {

/**
 * @brief Get file extension of file in provided path
 *
 * File is identified by last '/' delimiter in path.
 * Extension is identified by last '.' in File.
 * Extension is returned without '.'
 * @param path Path to file
 * @return std::string Extension without leading '.'
 */
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

/**
 * @brief Returns greenwhich meantime string in provided format.
 *
 * Uses strftime() to format the string. Provided format string should adhere this required format.
 * @param now Time for the string
 * @param format strftime format string
 * @return std::string Timestring in provided format
 */
std::string getGMTString(const time_t now, const char* format)
{
	char string[utils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format, gmtime(&now)));
	return string;
}

/**
 * @brief Returns localtime string in provided format.
 *
 * Uses strftime() to format the string. Provided format string should adhere this required format.
* @param now Time for the string
 * @param format strftime format string
 * @return std::string Timestring in provided format
 */
std::string getLocaltimeString(const time_t now, const char* format)
{
	char string[utils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format, localtime(&now)));
	return string;
}

} // namespace utils
