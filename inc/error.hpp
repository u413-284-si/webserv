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
#define ERR_NONSUPPORTED_VERSION "Invalid HTTP request: version not supported"

// HTTP REQUEST HEADER ERRORS
#define ERR_HEADER_COLON_WHITESPACE "Invalid HTTP request: Whitespace between header field-name and colon detected"
#define ERR_HEADER_NAME_INVALID_CHAR "Invalid HTTP request: Invalid char in header field name"
#define ERR_MISSING_HOST_HEADER "Invalid HTTP request: Missing host header"
#define ERR_EMPTY_HOST_VALUE "Invalid HTTP request: Missing hostname"
#define ERR_MULTIPLE_HOST_HEADERS "Invalid HTTP request: Multiple host headers"
#define ERR_INVALID_HOSTNAME "Invalid HTTP request: Invalid hostname"
#define ERR_INVALID_HOST_IP "Invalid HTTP request: Invalid IP as hostname"
#define ERR_INVALID_HOST_IP_WITH_PORT "Invalid HTTP request: Invalid IP with port as hostname"
#define ERR_MULTIPLE_CONTENT_LENGTH_VALUES "Invalid HTTP request: Multiple differing content-length values"
#define ERR_INVALID_CONTENT_LENGTH "Invalid HTTP request: Invalid content-length provided"
#define ERR_NON_FINAL_CHUNKED_ENCODING "Invalid HTTP request: Chunked encoding not the final encoding"
#define ERR_NON_EXISTENT_TRANSFER_ENCODING "Invalid HTTP request: Transfer encoding not detected"
#define ERR_BAD_MULTIPART_FORMDATA "Invalid HTTP request: Multipart/form-data invalid"

// HTTP REQUEST BODY ERRORS
#define ERR_NON_EXISTENT_CHUNKSIZE "Invalid HTTP request: Chunk size not detected"
#define ERR_INVALID_HEX_CHAR "Invalid HTTP request: Invalid hex character detected"
#define ERR_CONVERSION_STRING_TO_HEX "String to hex conversion error"
#define ERR_CHUNK_SIZE "Invalid HTTP request: Indicated chunk size different than actual chunk size"
#define ERR_CONVERSION_STRING_TO_SIZE_T "String to size_t conversion error"
#define ERR_CONTENT_LENGTH "Invalid HTTP request: Indicated content length different than actual body size"
#define ERR_UNEXPECTED_BODY "Invalid HTTP request: Method should not have a body"
