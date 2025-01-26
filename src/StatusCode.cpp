#include "StatusCode.hpp"

/**
 * @brief Converts a status code to a string.
 *
 * If the status code is not a known status code, it returns "0".
 * @param statusCode Status code.
 * @return std::string String representation of the status code.
 */
std::string statusCodeToString(statusCode statusCode)
{
	if (statusCode < NoStatus || statusCode > StatusNonSupportedVersion)
		statusCode = NoStatus;

	switch (statusCode) {
	case NoStatus:
		return ("0");
	case StatusOK:
		return ("200");
	case StatusCreated:
		return ("201");
	case StatusMovedPermanently:
		return ("301");
	case StatusFound:
		return ("302");
	case StatusPermanentRedirect:
		return ("308");
	case StatusBadRequest:
		return ("400");
	case StatusForbidden:
		return ("403");
	case StatusNotFound:
		return ("404");
	case StatusMethodNotAllowed:
		return ("405");
	case StatusRequestTimeout:
		return ("408");
	case StatusRequestEntityTooLarge:
		return ("413");
	case StatusRequestHeaderFieldsTooLarge:
		return ("431");
	case StatusInternalServerError:
		return ("500");
	case StatusMethodNotImplemented:
		return ("501");
	case StatusNonSupportedVersion:
		return ("505");
	}
	return ("0");
}

/**
 * @brief Returns reason phrase for a given status code.
 *
 * If the status code is not known returns the string "NO STATUS CODE".
 * @param statusCode Status code.
 * @return std::string Reason phrase.
 */
std::string statusCodeToReasonPhrase(statusCode statusCode)
{
	if (statusCode < NoStatus || statusCode > StatusNonSupportedVersion)
		statusCode = NoStatus;

	switch (statusCode) {
	case NoStatus:
		return "NO STATUS CODE";
	case StatusOK:
		return "OK";
	case StatusCreated:
		return "Created";
	case StatusMovedPermanently:
		return "Moved Permanently";
	case StatusFound:
		return "Found";
	case StatusPermanentRedirect:
		return "Permanent Redirect";
	case StatusBadRequest:
		return "Bad Request";
	case StatusForbidden:
		return "Forbidden";
	case StatusNotFound:
		return "Not Found";
	case StatusRequestEntityTooLarge:
		return "Request Entity Too Large";
	case StatusMethodNotAllowed:
		return "Method Not Allowed";
	case StatusRequestTimeout:
		return "Request Timeout";
	case StatusRequestHeaderFieldsTooLarge:
		return "Request Header Fields Too Large";
	case StatusInternalServerError:
		return "Internal Server Error";
	case StatusMethodNotImplemented:
		return "Not Implemented";
	case StatusNonSupportedVersion:
		return "HTTP Version Not Supported";
	}
	return "NO STATUS CODE";
}

/**
 * @brief Converts a string to an HTTP status code.
 *
 * Expects the string to be exactly 3 numbers long. If the string is not a valid status code, it returns NoStatus.
 * @param str The string representation of the HTTP status code.
 * @return The corresponding statusCode enum value.
 */
statusCode stringToStatusCode(const std::string& str)
{
	if (str.size() != 3)
		return (NoStatus);

	char* endptr = NULL;
	statusCode statusCode = static_cast<enum statusCode>(std::strtol(str.c_str(), &endptr, constants::g_decimalBase));
	if (*endptr != '\0')
		statusCode = NoStatus;

	switch (statusCode) {
	case NoStatus:
	case StatusOK:
	case StatusCreated:
	case StatusMovedPermanently:
	case StatusFound:
	case StatusPermanentRedirect:
	case StatusBadRequest:
	case StatusForbidden:
	case StatusNotFound:
	case StatusRequestEntityTooLarge:
	case StatusMethodNotAllowed:
	case StatusRequestTimeout:
	case StatusRequestHeaderFieldsTooLarge:
	case StatusInternalServerError:
	case StatusMethodNotImplemented:
	case StatusNonSupportedVersion:
		return (statusCode);
	}
	return (NoStatus);
}

/**
 * @brief Extracts the status code from a status line.
 *
 * Extratcs from the first number found till the last number.
 * @param statusLine The status line.
 * @return The status code.
 */
statusCode extractStatusCode(const std::string& statusLine)
{
	size_t pos = statusLine.find_first_of("0123456789");
	if (pos != std::string::npos) {
		std::string statusCodeString = statusLine.substr(pos, statusLine.find_first_not_of("0123456789", pos) - pos);
		return stringToStatusCode(statusCodeString);
	}
	return NoStatus;
}

/**
 * @brief Check if a given status code is an error status code.
 *
 * An error status code is a 3xx, 4xx or 5xx status code.
 * @param statusCode Status code to check.
 * @return true If the status code is an error status code.
 * @return false If the status code is not an error status code.
 */
bool isErrorStatus(statusCode statusCode) { return (statusCode >= StatusMovedPermanently); }

/**
 * @brief Check if a given status code is a redirection.
 *
 * A redirection is a 3xx status code.
 * @param statusCode Status code to check.
 * @return true If the status code is a redirection.
 * @return false If the status code is not a redirection.
 */
bool isRedirectionStatus(statusCode statusCode)
{
	return (statusCode >= StatusMovedPermanently && statusCode <= StatusPermanentRedirect);
}

/**
 * @brief Check if a given status code closes connection
 *
 * Connection: close is sent with status:
 * - 400 Bad Request
 * - 405 Method Not Allowed
 * - 408 Request Timeout
 * - 413 Request Entity Too Large
 * - 431 Request Header Fields Too Large
 * @param statusCode Status code to check.
 * @return true If status code closes connection
 * @return false If status code doesn't close connection
 */
bool isCloseConnectionStatus(statusCode statusCode)
{
	switch (statusCode) {
	case StatusBadRequest:
	case StatusMethodNotAllowed:
	case StatusRequestTimeout:
	case StatusRequestEntityTooLarge:
	case StatusRequestHeaderFieldsTooLarge:
		return true;
	default:
		return false;
	}
}
