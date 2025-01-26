#include "TargetResourceHandler.hpp"

/**
 * @brief Construct a new TargetResourceHandler object
 *
 * @param fileSystemOps Wrapper for filesystem-related functions. Can be mocked if needed.
 */
TargetResourceHandler::TargetResourceHandler(const FileSystemOps& fileSystemOps)
	: m_fileSystemOps(fileSystemOps)
{
}

/**
 * @brief Determine target resource based on the request path and handle different file types and statuses accordingly.
 *
 * Constructs a new LocatingInfo object with the provided Connection.
 * Recursively searches for the target resource with locateTargetResource() using the information in the LocatingInfo.
 * Uses updateConnection() to write found information into Connection.
 * @param request Request object to update.
 * @param location Iterator to the currently active location block, which gets updated.
 * @param serverConfig Iterator to the currently active server configuration.
 * @throws std::runtime_error If no location block is found for the path.
 */
void TargetResourceHandler::execute(HTTPRequest& request, std::vector<Location>::const_iterator& location, std::vector<ConfigServer>::const_iterator serverConfig)
{
	LocatingInfo locInfo(request, location);
	const int startingDepth = 0;
	locInfo = locateTargetResource(locInfo, startingDepth, serverConfig->locations);
	updateRequestAndLocation(request, location, locInfo);
}

/**
 * @brief Locates the target resource and checks its type.
 *
 * This function is called recursively. To exit recursion it keeps track of recursion depth.
 * Checks which location block most closely matches path with matchLocation().
 * If the found location has a return directive, set Status to the return status and set target resource to the return
 * value. Exits.
 * Else constructs target resource by appending the path to location root.
 *
 * Determines the type with FileSystemPolicy::checkFileType() and take different action:
 * - FileRegular: break, nothing more to do.
 * - FileDirectory: further action with handleFileDirectory()
 * - FileOther: set Status to StatusInternalServerError.
 * If FileSystemPolicy::checkFileType() throws, sets status depending on exception type:
 * - FileSystemPolicy::FileNotFoundException: StatusNotFound
 * - FileSystemPolicy::NoPermissionException: StatusForbidden
 * - std::runtime_error: StatusInternalServerError.
 * @param locInfo Struct containing info for locating the target resource.
 * @param depth Current depth in recursion.
 * @param locations Vector of Location objects.
 * @return TargetResourceHandler::LocatingInfo
 */
// NOLINTNEXTLINE (misc-no-recursion): recursion is being handled
TargetResourceHandler::LocatingInfo TargetResourceHandler::locateTargetResource(LocatingInfo locInfo, int depth, const std::vector<Location>& locations)
{
	const int currentDepth = depth + 1;
	if (currentDepth == s_maxRecursion) {
		LOG_ERROR << "Max recursion reached: " << currentDepth;
		locInfo.statusCode = StatusInternalServerError;
		return (locInfo);
	}

	locInfo.activeLocation = matchLocation(locations, locInfo.path);

	LOG_DEBUG << "Active location: " << locInfo.activeLocation->path;

	if (locInfo.activeLocation->returns.first != NoStatus) {
		LOG_DEBUG << "Location returns [" << locInfo.activeLocation->returns.first
				  << "]: " << locInfo.activeLocation->returns.second;
		locInfo.hasReturn = true;
		locInfo.statusCode = locInfo.activeLocation->returns.first;
		locInfo.targetResource = locInfo.activeLocation->returns.second;
		return (locInfo);
	}

	if (!locInfo.activeLocation->alias.empty()) {
		LOG_DEBUG << "Alias found: " << locInfo.activeLocation->alias;
		locInfo.targetResource
			= webutils::replaceAlias(locInfo.path, locInfo.activeLocation->path, locInfo.activeLocation->alias);
	} else
		locInfo.targetResource = locInfo.activeLocation->root + locInfo.path;
	LOG_DEBUG << "Target resource: " << locInfo.targetResource;

	try {
		FileSystemOps::fileType fileType = m_fileSystemOps.checkFileType(locInfo.targetResource);

		switch (fileType) {

		case FileSystemOps::FileRegular:
			LOG_DEBUG << "File type: regular";
			break;

		case FileSystemOps::FileDirectory:
			LOG_DEBUG << "File type: directory";
			handleFileDirectory(locInfo, currentDepth, locations);
			break;

		case FileSystemOps::FileOther:
			LOG_DEBUG << "File type: other";
			locInfo.statusCode = StatusForbidden;
			break;

		case FileSystemOps::FileNotFound:
			LOG_DEBUG << "File not found";
			locInfo.statusCode = StatusNotFound;
		}
	} catch (FileSystemOps::NoPermissionException& e) {
		LOG_ERROR << e.what();
		locInfo.statusCode = StatusForbidden;
	} catch (const std::runtime_error& e) {
		LOG_ERROR << e.what();
		locInfo.statusCode = StatusInternalServerError;
	}
	return (locInfo);
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
	std::vector<Location>::const_iterator locationMatch = locations.begin();

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

/**
 * @brief Construct a new LocatingInfo object
 *
 * @param request Request object from which to extract info.
 * @param location Iterator to the currently active location block.
 */
TargetResourceHandler::LocatingInfo::LocatingInfo(const HTTPRequest& request, const std::vector<Location>::const_iterator location)
	: statusCode(request.httpStatus)
	, path(request.uri.path)
	, isDirectory(false)
	, hasAutoindex(false)
	, hasReturn(false)
	, activeLocation(location)
{
}

/**
 * @brief Writes info from LocatingInfo into Connection
 *
 * @param request Request object which is updated with info.
 * @param location Iterator to the currently active location block.
 * @param locInfo LocationInfo object from which to extract info.
 */
void TargetResourceHandler::updateRequestAndLocation(HTTPRequest& request, std::vector<Location>::const_iterator& location, const LocatingInfo& locInfo)
{
	request.httpStatus = locInfo.statusCode;
	request.uri.path = locInfo.path;
	request.targetResource = locInfo.targetResource;
	request.isDirectory = locInfo.isDirectory;
	request.hasAutoindex = locInfo.hasAutoindex;
	request.hasReturn = locInfo.hasReturn;
	location = locInfo.activeLocation;
}

/**
 * @brief Handles filetype directory while locating target resource.
 *
 * This function is called within a recursive call chain.
 * If the found file is of type directory different actions can happen:
 * - if no trailing slash, add it and set Status to StatusMovedPermanently.
 * - if index vector is not empty, try to locate a index file with locateIndexFile().
 * - if autoindex is set, set hasAutoindex to true.
 * - else set Status to StatusForbidden.
 * If the file is a directory and no index is appended, isDirectory is set to true.
 * @param locInfo Reference to LocationInfo which gets overwritten.
 * @param currentDepth Current depth in recursion.
 * @param locations Vector of Location objects.
 */
// NOLINTNEXTLINE
void TargetResourceHandler::handleFileDirectory(LocatingInfo& locInfo, int currentDepth, const std::vector<Location>& locations)
{
	if (locInfo.targetResource.at(locInfo.targetResource.length() - 1) != '/') {
		locInfo.targetResource = locInfo.path + "/";
		locInfo.isDirectory = true;
		locInfo.statusCode = StatusMovedPermanently;
		return;
	}

	if (!locInfo.activeLocation->indices.empty()) {
		if (locateIndexFile(locInfo, currentDepth, locations))
			return;
	}

	if (locInfo.activeLocation->hasAutoindex) {
		locInfo.isDirectory = true;
		locInfo.hasAutoindex = true;
		return;
	}

	locInfo.isDirectory = true;
	locInfo.statusCode = StatusForbidden;
}

/**
 * @brief Finds index file
 *
 * This function is called within a recursive call chain.
 * Saves current path to be able to reset it.
 * Loops through the index vector of the active location.
 * Appends the index to path and tries to find it with locateTargetResource(), which is saved in a copy of locInfo.
 * If status code is not StatusNotFound it indicates that the file was found or an error occured. The passed locInfo is
 * overwritten with the copy.
 * Likewise if hasReturn was set to true the location had a Return directive, ending the search.
 * Else the path is reset to the saved path and the next index file is tried.
 * At the end compares the saved path with the path in locInfo, which indicates if an index was found.
 * @param locInfo Reference to LocationInfo which gets overwritten.
 * @param currentDepth Current depth in recursion.
 * @param locations Vector of Location objects.
 * @return true If an index file was found, false otherwise
 */
// NOLINTNEXTLINE
bool TargetResourceHandler::locateIndexFile(LocatingInfo& locInfo, int currentDepth, const std::vector<Location>& locations)
{
	std::string savePath = locInfo.path;
	for (std::vector<std::string>::const_iterator iter = locInfo.activeLocation->indices.begin();
		 iter != locInfo.activeLocation->indices.end(); ++iter) {
		locInfo.path += *iter;
		LocatingInfo tmp = locateTargetResource(locInfo, currentDepth, locations);
		if (tmp.statusCode != StatusNotFound && !tmp.hasReturn) {
			locInfo = tmp;
			break;
		}
		locInfo.path = savePath;
	}
	return (locInfo.path != savePath);
}
