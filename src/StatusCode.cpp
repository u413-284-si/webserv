#include "StatusCode.hpp"

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
}

/**
 * @brief Returns reason phrase for a given status code.
 *
 * In case of NoStatus returns the string "NO STATUS CODE".
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
 * @brief Converts a string to an HTTP status code.
 *
 * This function takes a string representation of an HTTP status code
 * and returns the corresponding statusCode enum value. If the string
 * does not match any known status code, it returns StatusBadRequest.
 *
 * @param str The string representation of the HTTP status code.
 * @return The corresponding statusCode enum value.
 */
statusCode stringToStatusCode(std::string& str)
{
	if (str == "0")
		return NoStatus;
	if (str == "200")
		return StatusOK;
	if (str == "201")
		return StatusCreated;
	if (str == "301")
		return StatusMovedPermanently;
	if (str == "302")
		return StatusFound;
	if (str == "308")
		return StatusPermanentRedirect;
	if (str == "400")
		return StatusBadRequest;
	if (str == "403")
		return StatusForbidden;
	if (str == "404")
		return StatusNotFound;
	if (str == "405")
		return StatusMethodNotAllowed;
	if (str == "408")
		return StatusRequestTimeout;
	if (str == "413")
		return StatusRequestEntityTooLarge;
	if (str == "431")
		return StatusRequestHeaderFieldsTooLarge;
	if (str == "500")
		return StatusInternalServerError;
	if (str == "501")
		return StatusMethodNotImplemented;
	if (str == "505")
		return StatusNonSupportedVersion;

	return NoStatus;
}

statusCode extractStatusCode(const std::string& statusLine)
{
	size_t pos = statusLine.find_first_of("0123456789");
	if (pos != std::string::npos) {
		std::string statusCodeString = statusLine.substr(pos, statusLine.find_first_not_of("0123456789", pos) - pos);
		return stringToStatusCode(statusCodeString);
	}
	return NoStatus;
}
