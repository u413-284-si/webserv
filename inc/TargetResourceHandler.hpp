#pragma once

#include <string>
#include <vector>

#include "ConfigFile.hpp"
#include "RequestParser.hpp"
#include "StatusCode.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"

class TargetResourceHandler {

public:
	TargetResourceHandler(const std::vector<Location>& locations, const FileSystemPolicy& fileSystemPolicy);
	HTTPResponse execute(const HTTPRequest& request);

private:
	std::vector<Location>::const_iterator matchLocation(const std::string& path);

	std::vector<Location> m_locations;
	const FileSystemPolicy& m_fileSystemPolicy;
	HTTPResponse m_response;
};

