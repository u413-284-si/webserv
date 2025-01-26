#pragma once

#include <string>
#include <vector>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "FileSystemOps.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "StatusCode.hpp"

/**
 * @brief Class to handle the target resource of a HTTP Request.
 *
 * The FileSystemOps passed in the constructor needs to outlive this class.
 * A mock of FileSystemOps can be used for testing.
 */
class TargetResourceHandler {

public:
	static const int s_maxRecursion = 10; /**< Max recursion depth for locating a resource */

	explicit TargetResourceHandler(const FileSystemOps& fileSystemOps);
	void execute(HTTPRequest& request, std::vector<Location>::const_iterator& location, std::vector<ConfigServer>::const_iterator serverConfig);

private:
	struct LocatingInfo;

	LocatingInfo locateTargetResource(LocatingInfo locInfo, int depth, const std::vector<Location>& locations);
	void handleFileDirectory(LocatingInfo& locInfo, int currentDepth, const std::vector<Location>& locations);
	bool locateIndexFile(LocatingInfo& locInfo, int currentDepth, const std::vector<Location>& locations);
	static void updateRequestAndLocation(HTTPRequest& request, std::vector<Location>::const_iterator& location, const LocatingInfo& locInfo);

	const FileSystemOps& m_fileSystemOps;
};

/**
 * @brief Helper struct to extract information out of Connection
 *
 * This struct is used to extract the needed information out of the Connection struct. It then can easier be passed
 * around and copied.
 */
struct TargetResourceHandler::LocatingInfo {
	explicit LocatingInfo(const HTTPRequest& request, std::vector<Location>::const_iterator location);

	statusCode statusCode;
	std::string path;
	std::string targetResource;
	bool isDirectory;
	bool hasAutoindex;
	bool hasReturn;
	std::vector<Location>::const_iterator activeLocation;
};

std::vector<Location>::const_iterator matchLocation(const std::vector<Location>& locations, const std::string& path);
