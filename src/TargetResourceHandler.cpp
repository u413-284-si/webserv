#include "TargetResourceHandler.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"

TargetResourceHandler::TargetResourceHandler(const std::vector<Location>& locations, const FileSystemPolicy& fileSystemPolicy)
	: m_locations(locations)
	, m_fileSystemPolicy(fileSystemPolicy)
{
	m_response.status = StatusOK;
}

HTTPResponse TargetResourceHandler::execute(const HTTPRequest& request)
{
	bool internalRedirect = false;

	do {
		// Check which location block matches the path
		m_response.location = matchLocation(request.uri.path);

		// No location found > do we also set a default location to not make extra check?
		if (m_response.location == m_locations.end()) {
			m_response.status = StatusNotFound;
			break;
		}

		// construct target resource
		if (!internalRedirect)
			m_response.targetResource = m_response.location->root + request.uri.path;
		internalRedirect = false;

		// what type is it
		FileSystemPolicy::fileType fileType = m_fileSystemPolicy.checkFileType(m_response.targetResource);
		switch (fileType) {

		case FileSystemPolicy::FileRegular:
			break;

		case FileSystemPolicy::FileDirectory:
			if (m_response.targetResource.at(m_response.targetResource.length() - 1) != '/') {
				m_response.targetResource += "/";
				m_response.status = StatusMovedPermanently;
			} else {
				m_response.targetResource += m_response.location->index;
				internalRedirect = true;
			}
			break;

		case FileSystemPolicy::FileNotExist:
			m_response.status = StatusNotFound;
			break;

		case FileSystemPolicy::FileOther:
			m_response.status = StatusForbidden;
			break;

		case FileSystemPolicy::StatError:
			m_response.status = StatusInternalServerError;
			break;
		}
	} while (internalRedirect);

	return m_response;
}

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
