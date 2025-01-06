#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
	: method(MethodCount)
	, contentLength(0)
	, httpStatus(StatusOK)
	, shallCloseConnection(false)
	, hasBody(false)
	, isChunked(false)
	, isDirectory(false)
	, hasAutoindex(false)
	, hasCGI(false)
	, hasReturn(false)
    , hasMultipartFormdata(false)
{
}
