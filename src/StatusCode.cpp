#include "StatusCode.hpp"

std::string statusCodeToString(statusCode status)
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
