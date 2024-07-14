#include "StatusCode.hpp"

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

std::string getDefaultErrorPage(statusCode status)
{
	static char error301Page[] = "<html>\r\n"
								 "<head><title>301 Moved permanently</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>301 Moved permanently</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	static char error400Page[] = "<html>\r\n"
								 "<head><title>400 Bad request</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>400 Bad request</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	static char error403Page[] = "<html>\r\n"
								 "<head><title>403 Forbidden</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>403 Forbidden</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	static char error404Page[] = "<html>\r\n"
								 "<head><title>404 Not Found</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>404 Not Found</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	static char error405Page[] = "<html>\r\n"
								 "<head><title>405 Method not allowed</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>405 Method not allowed</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	static char error500page[] = "<html>\r\n"
								 "<head><title>500 Internal server error</title></head>\r\n"
								 "<body>\r\n"
								 "<center><h1>500 Internal server error</h1></center>\r\n"
								 "<hr><center>TriHard</center>\r\n"
								 "</body>\r\n"
								 "</html>\r\n";

	switch (status) {
	case StatusOK:
		return ("");
	case StatusMovedPermanently:
		return (error301Page);
	case StatusBadRequest:
		return (error400Page);
	case StatusForbidden:
		return (error403Page);
	case StatusNotFound:
		return (error404Page);
	case StatusMethodNotAllowed:
		return (error405Page);
	case StatusInternalServerError:
		return (error500page);
	default:
		return ("");
	}
}
