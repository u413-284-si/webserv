#pragma once

#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"

class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(const FileSystemPolicy& fileSystemPolicy);
	HTTPResponse execute(HTTPResponse& response);

private:
	void handleErrorBody(HTTPResponse& response);

	const FileSystemPolicy& m_fileSystemPolicy;
};
