#pragma once

#include <string>
#include <vector>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPRequest.hpp"
#include "Log.hpp"
#include "StatusCode.hpp"

/**
 * @brief Class to handle the target resource of a HTTP Request.
 *
 * The FileSystemPolicy passed in the constructor needs to outlive this class.
 * A mock of FileSystemPolicy can be used for testing.
 */
class TargetResourceHandler {

public:
	static const int s_maxRecursion = 10; /**< Max recursion depth for locating a resource */

	explicit TargetResourceHandler(const FileSystemPolicy& fileSystemPolicy);
	void execute(Connection& connection, HTTPRequest& request);

private:
	struct LocatingInfo;

	LocatingInfo locateTargetResource(LocatingInfo locInfo, int depth);
	void handleFileDirectory(LocatingInfo& locInfo, int currentDepth);
	bool locateIndexFile(LocatingInfo& locInfo, int currentDepth);
	static void updateConnection(Connection& connection, const LocatingInfo& locInfo);

	const FileSystemPolicy& m_fileSystemPolicy;
};

/**
 * @brief Helper struct to extract information out of Connection
 *
 * This struct is used to extract the needed information out of the Connection struct. It then can easier be passed
 * around and copied.
 */
struct TargetResourceHandler::LocatingInfo {
	explicit LocatingInfo(const Connection& connection);

	statusCode statusCode;
	std::string path;
	std::string targetResource;
	bool hasAutoindex;
	const std::vector<Location>* locations;
	std::vector<Location>::const_iterator activeLocation;
};

std::vector<Location>::const_iterator matchLocation(const std::vector<Location>& locations, const std::string& path);
