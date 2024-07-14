#include "TargetResourceHandler.hpp"
#include "FileSystemPolicy.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"

TargetResourceHandler::TargetResourceHandler(const std::vector<Location>& locations, const HTTPRequest& request,
	HTTPResponse& response, const FileSystemPolicy& fileSystemPolicy)
	: m_locations(locations)
	, m_request(request)
	, m_response(response)
	, m_fileSystemPolicy(fileSystemPolicy)
{
}

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

		case FileSystemPolicy::StatError:
			m_response.status = StatusInternalServerError;
			break;
		}
	} while (internalRedirect);
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
