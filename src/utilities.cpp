#include "utilities.hpp"
#include "StatusCode.hpp"
#include <cctype>
#include <sys/socket.h>

namespace webutils {

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
std::string trimLeadingWhitespaces(const std::string& str)
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
void trimTrailingWhiteSpaces(std::string& str)
{
	std::string::size_type end = str.size();

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
std::vector<std::string> split(const std::string& str, const std::string& delimiter)
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
	if (dirPos == std::string::npos)
		dirPos = 0;

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
std::string getGMTString(const time_t now, const std::string& format)
{
	char string[webutils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format.c_str(), gmtime(&now)));
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
std::string getLocaltimeString(const time_t now, const std::string& format)
{
	char string[webutils::timeStringBuffer];

	static_cast<void>(strftime(string, sizeof(string), format.c_str(), localtime(&now)));
	return string;
}

/**
 * @brief Returns reason phrase for a given status code.
 *
 * @param status Status code.
 * @return std::string Reason phrase.
 */
std::string statusCodeToReasonPhrase(statusCode status)
{
	switch (status) {
	case StatusOK:
		return "OK";
	case StatusMovedPermanently:
		return "Moved Permanently";
	case StatusBadRequest:
		return "Bad Request";
	case StatusForbidden:
		return "Forbidden";
	case StatusNotFound:
		return "Not Found";
	case StatusRequestTooLarge:
		return "Request Entity Too Large";
	case StatusMethodNotAllowed:
		return "Method Not Allowed";
	case StatusInternalServerError:
		return "Internal Server Error";
	default:
		return "Unknown";
	}
}

/**
 * @brief Get Default Error Page for a given status code.
 *
 * @param status Status code.
 * @return std::string Default error page.
 */
std::string getDefaultErrorPage(statusCode status)
{
	static const char* error301Page = "<html>\r\n"
									  "<head><title>301 Moved permanently</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>301 Moved permanently</h1></center>\r\n";

	static const char* error400Page = "<html>\r\n"
									  "<head><title>400 Bad request</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>400 Bad request</h1></center>\r\n";

	static const char* error403Page = "<html>\r\n"
									  "<head><title>403 Forbidden</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>403 Forbidden</h1></center>\r\n";

	static const char* error404Page = "<html>\r\n"
									  "<head><title>404 Not Found</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>404 Not Found</h1></center>\r\n";

	static const char* error405Page = "<html>\r\n"
									  "<head><title>405 Method not allowed</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>405 Method not allowed</h1></center>\r\n";

	static const char* error413Page = "<html>\r\n"
									  "<head><title>413 Request Entity Too Large</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>413 Request Entity Too Large</h1></center>\r\n";

	static const char* error500page = "<html>\r\n"
									  "<head><title>500 Internal server error</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>500 Internal server error</h1></center>\r\n";

	static const char* error501page = "<html>\r\n"
									  "<head><title>501 Method not implemented</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>501 Method not implemented</h1></center>\r\n";

	static const char* error505page = "<html>\r\n"
									  "<head><title>505 Non supported version</title></head>\r\n"
									  "<body>\r\n"
									  "<center><h1>505 Non supported version</h1></center>\r\n";

	static const char* errorTail = "<hr><center>TriHard</center>\r\n"
								   "</body>\r\n"
								   "</html>\r\n";

	std::string ret;

	switch (status) {
	case StatusOK:
		return ("");
	case StatusMovedPermanently:
		ret = error301Page;
		break;
	case StatusBadRequest:
		ret = error400Page;
		break;
	case StatusForbidden:
		ret = error403Page;
		break;
	case StatusNotFound:
		ret = error404Page;
		break;
	case StatusRequestTooLarge:
		ret = error413Page;
		break;
	case StatusMethodNotAllowed:
		ret = error405Page;
		break;
	case StatusInternalServerError:
		ret = error500page;
		break;
	case StatusMethodNotImplemented:
		ret = error501page;
		break;
	case StatusNonSupportedVersion:
		ret = error505page;
		break;
	}

	ret += errorTail;
	return (ret);
}

} // webutils
