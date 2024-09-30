#include "TargetResourceHandler.hpp"

/**
 * @brief Construct a new TargetResourceHandler object
 *
 * @param fileSystemPolicy File system policy to be used. Can be mocked if needed.
 */
TargetResourceHandler::TargetResourceHandler(const FileSystemPolicy& fileSystemPolicy)
	: m_fileSystemPolicy(fileSystemPolicy)
{
}

/**
 * @brief Determine target resource based on the request path and handle different file types and statuses accordingly.
 *
 * Finds a matching location block.
 * If no match is found set Status to StatusNotFound.
 * Set the targetResource as location.root + request.uri.path and check file type.
 * RegularFile: break, nothing more to do.
 * Directory: if no trailing slash, add it and set Status to StatusMovedPermanently.
 *            if index file is set, add it to the path and set internalRedirect to true.
 *            if autoindex is set, set autoindex to true.
 *            else set Status to StatusForbidden.
 * FileNotExist: set Status to StatusNotFound.
 * FileOther: set Status to StatusForbidden.
 * If internalRedirect is true, repeat the process.
 * If stat fails, set Status to StatusInternalServerError.
 * @param connection Connection object.
 * @param request HTTP request.
 */
void TargetResourceHandler::execute(Connection& connection, HTTPRequest& request)
{
	(void)request;
	LocatingInfo locInfo(connection);
	const int startingDepth = 0;
	locInfo = locateTargetResource(locInfo, startingDepth);
	updateConnection(connection, locInfo);
}

TargetResourceHandler::LocatingInfo TargetResourceHandler::locateTargetResource(LocatingInfo locInfo, int depth)
{
	const int currentDepth = depth + 1;
	if (currentDepth == s_maxRecursion) {
		LOG_DEBUG << "Max recursion reached: " << currentDepth;
		locInfo.statusCode = StatusInternalServerError;
		return (locInfo);
	}

	// Check which location block matches the path
	locInfo.activeLocation = matchLocation(*locInfo.locations, locInfo.path);

	// No location found > do we also set a default location to not make extra check?
	if (locInfo.activeLocation == locInfo.locations->end()) {
		locInfo.statusCode = StatusInternalServerError;
		return (locInfo);
	}

	// construct target resource
	locInfo.targetResource = locInfo.activeLocation->root + locInfo.path;
	LOG_DEBUG << "Target resource: " << locInfo.targetResource;

	// what type is it
	try {
		FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(locInfo.targetResource);

		switch (fileType) {

		case FileSystemPolicy::FileRegular:
			break;

		case FileSystemPolicy::FileDirectory:
			handleFileDirectory(locInfo, currentDepth);
			break;

		case FileSystemPolicy::FileNotExist:
			locInfo.statusCode = StatusNotFound;
			break;

		case FileSystemPolicy::FileOther:
			locInfo.statusCode = StatusForbidden;
			break;
		}

		return (locInfo);

	} catch (const std::runtime_error& e) {
		LOG_ERROR << e.what();
		locInfo.statusCode = StatusInternalServerError;
		return (locInfo);
	}
}

/**
 * @brief Iterate through Location objects to find longest matching path based on a given input path.
 *
 * The longest matching path is the one that has the most characters in common with the input path.
 * If no match is found, the function returns an iterator pointing to the end of the vector.
 * @param locations Vector of Location objects.
 * @param path String representing the path to be matched.
 * @return Constant_iterator to the location that has the longest matching path with input.
 */
std::vector<Location>::const_iterator matchLocation(const std::vector<Location>& locations, const std::string& path)
{
	std::size_t longestMatch = 0;
	std::vector<Location>::const_iterator locationMatch = locations.end();

	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		if (path.find(it->path) == 0) {
			if (it->path.length() > longestMatch) {
				longestMatch = it->path.length();
				locationMatch = it;
			}
		}
	}
	return locationMatch;
}

TargetResourceHandler::LocatingInfo::LocatingInfo(const Connection& connection)
	: statusCode(connection.m_request.httpStatus)
	, path(connection.m_request.uri.path)
	, hasAutoindex(false)
	, locations(&connection.serverConfig->locations)
	, activeLocation(connection.location)
{
}

void TargetResourceHandler::updateConnection(Connection& connection, const LocatingInfo& locInfo)
{
	connection.m_request.httpStatus = locInfo.statusCode;
	connection.m_request.uri.path = locInfo.path;
	connection.m_request.targetResource = locInfo.targetResource;
	connection.m_request.hasAutoindex = locInfo.hasAutoindex;
	connection.location = locInfo.activeLocation;
}

bool TargetResourceHandler::locateIndexFile(LocatingInfo& locInfo, int currentDepth)
{
	std::string savePath = locInfo.path;
	for (std::vector<std::string>::const_iterator iter = locInfo.activeLocation->indices.begin();
		 iter != locInfo.activeLocation->indices.end(); ++iter) {
		locInfo.path += *iter;
		LocatingInfo tmp = locateTargetResource(locInfo, currentDepth);
		if (tmp.statusCode != StatusNotFound) {
			locInfo = tmp;
			break;
		}
		locInfo.path = savePath;
	}
	return (locInfo.path != savePath);
}

void TargetResourceHandler::handleFileDirectory(LocatingInfo& locInfo, int currentDepth)
{
	if (locInfo.targetResource.at(locInfo.targetResource.length() - 1) != '/') {
		locInfo.targetResource += "/";
		locInfo.statusCode = StatusMovedPermanently;
		return;
	}

	if (!locInfo.activeLocation->indices.empty()) {
		if (locateIndexFile(locInfo, currentDepth))
			return;
	}

	if (locInfo.activeLocation->isAutoindex) {
		locInfo.hasAutoindex = true;
		return;
	}

	locInfo.statusCode = StatusForbidden;
}
