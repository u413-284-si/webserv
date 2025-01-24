#pragma once

/* ====== DEFINITIONS ====== */

// PARSING ERRORS

#define ERR_CONFIG_FILE_OPEN_FAILED "Failed to open config file"
#define ERR_CONFIG_FILE_EMPTY "Config file is empty"
#define ERR_OPEN_BRACKET_IN_CONFIG_FILE "Open bracket(s) in config file"
#define ERR_MISSING_HTTP_BLOCK "Missing http block"
#define ERR_INVALID_DIRECTIVE "Invalid directive"
#define ERR_MISSING_SERVER_BLOCK "Missing server block(s)"
#define ERR_LOCATION_INVALID_BEGIN "Invalid location block begin"
#define ERR_DUPLICATE_LOCATION "Duplicate location"
#define ERR_SEMICOLON_MISSING "Unexpected '}'"
#define ERR_ROOT_AND_ALIAS_DEFINED "Defining root and alias in the same location block is not allowed"
#define ERR_MULTIPLE_ROOT_PATHS "More than one root path"
#define ERR_ROOT_PATH_MISSING_SLASH "Root path does not start with a slash"
#define ERR_MULTIPLE_ALIAS_PATHS "More than one alias path"
#define ERR_ALIAS_PATH_MISSING_SLASH "Alias path does not start with a slash"
#define ERR_MULTIPLE_SERVER_NAMES "More than one server name"
#define ERR_INVALID_IP_ADDRESS "Invalid ip address"
#define ERR_INVALID_PORT "Invalid port"
#define ERR_INVALID_LISTEN_PARAMETERS "Invalid amount of parameters for listen"
#define ERR_EMPTY_LISTEN_VALUE "'listen' value has no value"
#define ERR_INVALID_MAX_BODY_SIZE_PARAMETERS "Invalid amount of parameters for client_max_body_size"
#define ERR_INVALID_MAX_BODY_SIZE_VALUE "Invalid client_max_body_size value"
#define ERR_INVALID_MAX_BODY_SIZE_NUMBER "Invalid client_max_body_size number: Overflow"
#define ERR_INVALID_MAX_BODY_SIZE_UNIT "Invalid client_max_body_size unit"
#define ERR_INVALID_MAX_BODY_SIZE_NUMBER_OVERFLOW "Invalid client_max_body_size number: Overflow"
#define ERR_INVALID_MAX_BODY_SIZE_UNIT_OVERFLOW "Invalid client_max_body_size unit: Overflow"
#define ERR_INVALID_AUTOINDEX_PARAMETERS "Invalid amount of parameters for autoindex"
#define ERR_INVALID_AUTOINDEX_VALUE "Invalid autoindex value"
#define ERR_INVALID_ALLOW_METHODS "Invalid allow_methods value"
#define ERR_INVALID_ERROR_PAGE_PARAMS "Invalid amount of parameters for error_page"
#define ERR_INVALID_ERROR_CODE "Invalid error code"
#define ERR_ERROR_PAGE_NO_PATH "error_page directive path has no value"
#define ERR_ERROR_PAGE_PATH_NO_SLASH "Error page path does not start with a slash"
#define ERR_INVALID_RETURN_CODE "Invalid return code"
#define ERR_INVALID_RETURN_PARAMS "Invalid amount of parameters for return"
#define ERR_INVALID_CGI_EXTENSION "Invalid CGI extension"
#define ERR_MULTIPLE_CGI_EXTENSIONS "More than one CGI extension"
#define ERR_MULTIPLE_DOTS_IN_CGI_EXTENSION "More than one dot in CGI extension"
#define ERR_MULTIPLE_CGI_PATHS "More than one CGI path"
#define ERR_CGI_PATH_NO_SLASH "CGI path does not start with a slash"
#define ERR_INVALID_SERVER_DIRECTIVE "Invalid server directive"
#define ERR_INVALID_LOCATION_DIRECTIVE "Invalid location directive"
#define ERR_DIRECTIVE_NO_VALUE(directive) ("'" + (directive) + "' directive has no value")
#define ERR_TOO_MANY_DOUBLE_QUOTES "Text can only be enclosed within ONE pair of double quotes"

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
#define ERR_PERCENT_NONSUPPORTED_NUL "Invalid HTTP request: %00 (NUL) is not supported."
#define ERR_PERCENT_INCOMPLETE "Invalid HTTP request: Incomplete percent encoding at end of string."
#define ERR_PERCENT_INVALID_HEX "Invalid HTTP request: Percent encoding triplet consists of non hex values"
#define ERR_DIRECTORY_TRAVERSAL "Invalid HTTP request: request traverses outside of root"

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
#define ERR_TOO_LARGE_CHUNKSIZE "Invalid HTTP request: Chunk size too large"
#define ERR_INVALID_HEX_CHAR "Invalid HTTP request: Invalid hex character detected"
#define ERR_CONVERSION_STRING_TO_HEX "String to hex conversion error"
#define ERR_CHUNKSIZE_INCONSISTENT "Invalid HTTP request: Indicated chunk size different than actual chunk size"
#define ERR_CONVERSION_STRING_TO_SIZE_T "String to size_t conversion error"
#define ERR_CONTENT_LENGTH "Invalid HTTP request: Indicated content length different than actual body size"
#define ERR_UNEXPECTED_BODY "Invalid HTTP request: Method should not have a body"

// HTTP CGI RESPONSE HEADER ERRORS
#define ERR_MISSING_CGI_HEADER "Invalid CGI response: No header section found in CGI output"
#define ERR_MISSING_CGI_FIELD "Invalid CGI response: No CGI field (Content-type | Status | Location) found"
#define ERR_MULTIPLE_UPLOADS "Invalid HTTP request: Multiple file uploads not supported"
