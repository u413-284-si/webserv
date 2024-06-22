#pragma once

/* ====== DEFINITIONS ====== */

// HTTP REQUEST LINE ERRORS
#define ERR_MISS_SINGLE_SPACE "Invalid HTTP request: missing single space"
#define ERR_MISS_CRLF "Invalid HTTP request: missing CRLF"
#define ERR_MISS_REQUEST_LINE "Invalid HTTP request: missing request line"
#define ERR_OBSOLETE_LINE_FOLDING "Invalid HTTP request: Obsolete line folding detected"
#define ERR_METHOD_NOT_IMPLEMENTED "Invalid HTTP request: method not implemented"
#define ERR_URI_MISS_SLASH "Invalid HTTP request: missing slash in URI"
#define ERR_URI_INVALID_CHAR "Invalid HTTP request: invalid char in URI"
#define ERR_INVALID_VERSION_FORMAT "Invalid HTTP request: invalid format of version"
#define ERR_INVALID_VERSION_MAJOR "Invalid HTTP request: invalid version major"
#define ERR_INVALID_VERSION_DELIM "Invalid HTTP request: invalid version delimiter"
#define ERR_INVALID_VERSION_MINOR "Invalid HTTP request: invalid version minor"

// HTTP REQUEST HEADER ERRORS
#define ERR_HEADER_COLON_WHITESPACE "Invalid HTTP request: Whitespace between header field-name and colon detected"
#define ERR_HEADER_NAME_INVALID_CHAR "Invalid HTTP request: Invalid char in header field name"
#define ERR_MULTIPLE_CONTENT_LENGTH_VALUES "Invalid HTTP request: Multiple differing content-length values"
#define ERR_INVALID_CONTENT_LENGTH "Invalid HTTP request: Invalid content-length provided"
#define ERR_NON_FINAL_CHUNKED_ENCODING "Invalid HTTP request: Chunked encoding not the final encoding"
#define ERR_NON_EXISTENT_CHUNKED_ENCODING "Invalid HTTP request: Chunked encoding not detected"