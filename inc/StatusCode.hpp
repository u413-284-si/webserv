#pragma once

#include "constants.hpp"

#include <cstdlib>
#include <iostream>

enum statusCode {
	NoStatus = 0,
	StatusOK = 200,
	StatusCreated = 201,
	StatusMovedPermanently = 301,
	StatusFound = 302,
	StatusPermanentRedirect = 308,
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

std::string statusCodeToString(statusCode status);
std::string statusCodeToReasonPhrase(statusCode status);
statusCode stringToStatusCode(const std::string& str);
statusCode extractStatusCode(const std::string& statusLine);
bool isErrorStatus(statusCode statusCode);
bool isRedirectionStatus(statusCode statusCode);

std::ostream& operator<<(std::ostream& ostream, statusCode statusCode);
