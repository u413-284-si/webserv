#pragma once

#include <iostream>

enum statusCode {
	StatusOK = 200,
	StatusMovedPermanently = 301,
	StatusBadRequest = 400,
	StatusForbidden = 403,
	StatusNotFound = 404,
	StatusMethodNotAllowed = 405,
	StatusRequestTimeout = 408,
	StatusRequestEntityTooLarge = 413,
	StatusRequestHeaderFieldsTooLarge = 431,
	StatusInternalServerError = 500,
	StatusMethodNotImplemented = 501,
	StatusNonSupportedVersion = 505
};

std::ostream& operator<<(std::ostream& ostream, statusCode statusCode);
