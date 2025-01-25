#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
	: method(MethodCount)
	, chunkSize(-1)
	, contentLength(0)
	, httpStatus(StatusOK)
	, shallCloseConnection(false)
	, hasBody(false)
	, isCompleteBody(false)
	, isChunked(false)
	, isDirectory(false)
	, hasAutoindex(false)
	, hasCGI(false)
	, hasReturn(false)
	, hasMultipartFormdata(false)
{
}
