#include "TargetResourceHandler.hpp"

/**
 * @brief Construct a new TargetResourceHandler object
 *
 * @param locations Vector of Location objects of current server block.
 * @param request Request to handle.
 * @param response Response to be filled.
 * @param fileSystemPolicy File system policy to be used.
 */
TargetResourceHandler::TargetResourceHandler(const std::vector<Location>& locations, const HTTPRequest& request,
	HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy)
	: m_locations(locations)
	, m_request(request)
	, m_response(response)
	, m_fileSystemPolicy(fileSystemPolicy)
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
 */
void TargetResourceHandler::execute()
{
	bool internalRedirect = false;

	do {
		// Check which location block matches the path
		m_response.location = matchLocation(m_request.uri.path);

		// No location found > do we also set a default location to not make extra check?
		if (m_response.location == m_locations.end()) {
			m_response.status = StatusNotFound;
			break;
		}

		// construct target resource
		if (!internalRedirect)
			m_response.targetResource = m_response.location->root + m_request.uri.path;
		internalRedirect = false;

		// what type is it
		try {
			FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(m_response.targetResource);

			switch (fileType) {

			case FileSystemPolicy::FileRegular:
				break;

			case FileSystemPolicy::FileDirectory:
				if (m_response.targetResource.at(m_response.targetResource.length() - 1) != '/') {
					m_response.targetResource += "/";
					m_response.status = StatusMovedPermanently;
				} else if (!m_response.location->index.empty()) {
					m_response.targetResource += m_response.location->index;
					internalRedirect = true;
				} else if (m_response.location->isAutoindex) {
					m_response.autoindex = true;
				} else
					m_response.status = StatusForbidden;
				break;

			case FileSystemPolicy::FileNotExist:
				m_response.status = StatusNotFound;
				break;

			case FileSystemPolicy::FileOther:
				m_response.status = StatusForbidden;
				break;
			}
		} catch (const std::runtime_error& e) {
			std::cerr << "Stat error: " << e.what() << std::endl;
			m_response.status = StatusInternalServerError;
			break;
		}
	} while (internalRedirect);
}

/**
 * @brief Iterate through Location objects to find longest matching path based on a given input path.
 *
 * The longest matching path is the one that has the most characters in common with the input path.
 * If no match is found, the function returns an iterator pointing to the end of the vector.
 * @param path String representing the path to be matched.
 * @return Constant_iterator to the location that has the longest matching path with input.
 */
std::vector<Location>::const_iterator TargetResourceHandler::matchLocation(const std::string& path)
{
	std::size_t longestMatch = 0;
	std::vector<Location>::const_iterator locationMatch = m_locations.end();

	for (std::vector<Location>::const_iterator it = m_locations.begin(); it != m_locations.end(); ++it) {
		if (path.find(it->path) == 0) {
			if (it->path.length() > longestMatch) {
				longestMatch = it->path.length();
				locationMatch = it;
			}
		}
	}
	return locationMatch;
}
