#pragma once

#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"

class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy = FileSystemPolicy());
	void execute();

private:
	void handleErrorBody();

	HTTPResponse& m_response;
	const FileSystemPolicy& m_fileSystemPolicy;
};
