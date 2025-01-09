#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
	: method(MethodCount)
	, httpStatus(StatusOK)
	, shallCloseConnection(false)
	, hasBody(false)
	, isChunked(false)
	, isDirectory(false)
	, hasAutoindex(false)
	, hasCGI(false)
	, hasReturn(false)
	, hasCGIStatusBadRequest(false)
{
}
