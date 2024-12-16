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
 * Constructs a new LocatingInfo object with the provided Connection.
 * Recursively searches for the target resource with locateTargetResource() using the information in the LocatingInfo.
 * Uses updateConnection() to write found information into Connection.
 * @param connection Connection object.
 * @throws std::runtime_error If no location block is found for the path.
 */
void TargetResourceHandler::execute(Connection& connection)
{
	LocatingInfo locInfo(connection);
	const int startingDepth = 0;
	locInfo = locateTargetResource(locInfo, startingDepth);
	updateConnection(connection, locInfo);
}

/**
 * @brief Locates the target resource and checks its type.
 *
 * This function is called recursively. To exit recursion it keeps track of recursion depth.
 * Checks which location block most closely matches path with matchLocation().
 * Then constructs target resource by appending the path to location root.
 * Determines the type with stat() and take different action:
 * - FileRegular: break, nothing more to do.
 * - FileDirectory: further action with handleFileDirectory()
 * - FileNotExist: set Status to StatusNotFound.
 * - FileOther: set Status to StatusInternalServerError.
 * If stat fails, set Status to StatusInternalServerError.
 * @param locInfo Struct containing info for locating the target resource.
 * @param depth Current depth in recursion.
 * @return TargetResourceHandler::LocatingInfo
 * @throws std::runtime_error If no location block is found for the path.
 */
// NOLINTNEXTLINE (misc-no-recursion): recursion is being handled
TargetResourceHandler::LocatingInfo TargetResourceHandler::locateTargetResource(LocatingInfo locInfo, int depth)
{
	const int currentDepth = depth + 1;
	if (currentDepth == s_maxRecursion) {
		LOG_DEBUG << "Max recursion reached: " << currentDepth;
		locInfo.statusCode = StatusInternalServerError;
		return (locInfo);
	}

	locInfo.activeLocation = matchLocation(*locInfo.locations, locInfo.path);

	locInfo.targetResource = locInfo.activeLocation->root + locInfo.path;
	LOG_DEBUG << "Target resource: " << locInfo.targetResource;

	try {
		FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(locInfo.targetResource);

		switch (fileType) {

		case FileSystemPolicy::FileRegular:
			LOG_DEBUG << "File type: regular";
			break;

		case FileSystemPolicy::FileDirectory:
			LOG_DEBUG << "File type: directory";
			handleFileDirectory(locInfo, currentDepth);
			break;

		case FileSystemPolicy::FileOther:
			LOG_DEBUG << "File type: other";
			locInfo.statusCode = StatusForbidden;
			break;
		}
	} catch (FileSystemPolicy::FileNotFoundException& e) {
		LOG_ERROR << e.what();
		locInfo.statusCode = StatusNotFound;
	} catch (FileSystemPolicy::NoPermissionException& e) {
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
 * @param connection Connection object from which to extract info.
 */
TargetResourceHandler::LocatingInfo::LocatingInfo(const Connection& connection)
	: statusCode(connection.m_request.httpStatus)
	, path(connection.m_request.uri.path)
	, isDirectory(false)
	, hasAutoindex(false)
	, locations(&connection.serverConfig->locations)
	, activeLocation(connection.location)
{
}

/**
 * @brief Writes info from LocatingInfo into Connection
 *
 * @param connection Connection object which is updated with info.
 * @param locInfo LocationInfo object from which to extract info.
 */
void TargetResourceHandler::updateConnection(Connection& connection, const LocatingInfo& locInfo)
{
	connection.m_request.httpStatus = locInfo.statusCode;
	connection.m_request.uri.path = locInfo.path;
	connection.m_request.targetResource = locInfo.targetResource;
	connection.m_request.isDirectory = locInfo.isDirectory;
	connection.m_request.hasAutoindex = locInfo.hasAutoindex;
	connection.location = locInfo.activeLocation;
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
 */
// NOLINTNEXTLINE
void TargetResourceHandler::handleFileDirectory(LocatingInfo& locInfo, int currentDepth)
{
	if (locInfo.targetResource.at(locInfo.targetResource.length() - 1) != '/') {
		locInfo.targetResource += "/";
		locInfo.isDirectory = true;
		locInfo.statusCode = StatusMovedPermanently;
		return;
	}

	if (!locInfo.activeLocation->indices.empty()) {
		if (locateIndexFile(locInfo, currentDepth))
			return;
	}

	if (locInfo.activeLocation->isAutoindex) {
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
 * Else the path is reset to the saved path and the next index file is tried.
 * At the end compares the saved path with the path in locInfo, which indicates if an index was found.
 * @param locInfo Reference to LocationInfo which gets overwritten.
 * @param currentDepth Current depth in recursion.
 * @return true If an index file was found, false otherwise
 */
// NOLINTNEXTLINE
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
