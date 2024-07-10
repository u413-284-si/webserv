#pragma once

#include "FileHandler.hpp"
#include "HTTPResponse.hpp"

class ResponseBodyHandler {
public:
	explicit ResponseBodyHandler(const FileHandler& fileHandler);
	HTTPResponse execute(HTTPResponse& response);

private:
	void handleErrorBody(HTTPResponse& response);

	const FileHandler& m_fileHandler;
};
