#pragma once

#include <string>
#include <vector>

#include "ConfigFile.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "StatusCode.hpp"

/**
 * @brief Class to handle the target resource of a HTTP Request.
 *
 */
class TargetResourceHandler {

public:
	TargetResourceHandler(const std::vector<Location>& locations, const HTTPRequest& request, HTTPResponse& response,
		const FileSystemPolicy& fileSystemPolicy);
	void execute();

private:
	std::vector<Location>::const_iterator matchLocation(const std::string& path);

	const std::vector<Location>& m_locations;
	const HTTPRequest& m_request;
	HTTPResponse& m_response;
	const FileSystemPolicy& m_fileSystemPolicy;
};
