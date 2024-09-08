#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
	: method(MethodCount)
	, httpStatus(StatusOK)
	, shallCloseConnection(false)
	, hasBody(false)
	, isChunked(false)
	, hasAutoindex(false)
	, isCGI(false)
{
}
