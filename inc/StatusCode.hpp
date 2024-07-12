#pragma once

#include <string>

enum statusCode {
		StatusOK = 200,
		StatusMovedPermanently = 301,
		StatusBadRequest = 400,
		StatusForbidden = 403,
		StatusNotFound = 404,
		StatusMethodNotAllowed = 405,
		StatusInternalServerError = 500
};

std::string statusCodeToReasonPhrase(statusCode status);
std::string getDefaultErrorPage(statusCode status);
