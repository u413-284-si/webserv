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
	bool internalRedirect = false;

	do {
		// Check which location block matches the path
		connection.location = matchLocation(connection.serverConfig->locations, request.uri.path);

		// No location found > do we also set a default location to not make extra check?
		if (connection.location == connection.serverConfig->locations.end()) {
			request.httpStatus = StatusNotFound;
			break;
		}

		// construct target resource
		if (!internalRedirect)
			request.targetResource = connection.location->root + request.uri.path;
		internalRedirect = false;

		// what type is it
		try {
			FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(request.targetResource);

			switch (fileType) {

			case FileSystemPolicy::FileRegular:
				break;

			case FileSystemPolicy::FileDirectory:
				if (request.targetResource.at(request.targetResource.length() - 1) != '/') {
					request.targetResource += "/";
					request.httpStatus = StatusMovedPermanently;
				} else if (!connection.location->indices.empty()) {
					request.targetResource += connection.location->indices[0];
					internalRedirect = true;
				} else if (connection.location->isAutoindex) {
					request.hasAutoindex = true;
				} else
					request.httpStatus = StatusForbidden;
				break;

			case FileSystemPolicy::FileNotExist:
				request.httpStatus = StatusNotFound;
				break;

			case FileSystemPolicy::FileOther:
				request.httpStatus = StatusForbidden;
				break;
			}
		} catch (const std::runtime_error& e) {
			LOG_ERROR << e.what();
			request.httpStatus = StatusInternalServerError;
			break;
		}
	} while (internalRedirect);

	LOG_DEBUG << "Target resource: " << request.targetResource;
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
