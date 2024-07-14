#include "StatusCode.hpp"

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
	static const char* error301Page =
	"<html>\r\n"
	"<head><title>301 Moved permanently</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>301 Moved permanently</h1></center>\r\n";

	static const char* error400Page =
	"<html>\r\n"
	"<head><title>400 Bad request</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>400 Bad request</h1></center>\r\n";

	static const char* error403Page =
	"<html>\r\n"
	"<head><title>403 Forbidden</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>403 Forbidden</h1></center>\r\n";

	static const char* error404Page =
	"<html>\r\n"
	"<head><title>404 Not Found</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>404 Not Found</h1></center>\r\n";

	static const char* error405Page =
	"<html>\r\n"
	"<head><title>405 Method not allowed</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>405 Method not allowed</h1></center>\r\n";

	static const char* error500page =
	"<html>\r\n"
	"<head><title>500 Internal server error</title></head>\r\n"
	"<body>\r\n"
	"<center><h1>500 Internal server error</h1></center>\r\n";

	static const char* errorTail =
	"<hr><center>TriHard</center>\r\n"
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
	case StatusMethodNotAllowed:
		ret = error405Page;
		break;
	case StatusInternalServerError:
		ret = error500page;
		break;
	}

	ret += errorTail;
	return (ret);
}
