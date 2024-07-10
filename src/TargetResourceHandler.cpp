#include "TargetResourceHandler.hpp"
#include "FileHandler.hpp"
#include "HTTPResponse.hpp"
#include "RequestParser.hpp"

TargetResourceHandler::TargetResourceHandler(const std::vector<Location>& locations, const FileHandler& fileHandler)
	: m_locations(locations)
	, m_fileHandler(fileHandler)
{
	m_response.status = StatusOK;
}

HTTPResponse TargetResourceHandler::execute(const HTTPRequest& request)
{
	bool internalRedirect = false;

	do {
		// Check which location block matches the path
		std::vector<Location>::const_iterator locationMatch = matchLocation(request.uri.path);

		// No location found > do we also set a default location to not make extra check?
		if (locationMatch == m_locations.end()) {
			m_response.status = StatusNotFound;
			break;
		}

		// construct target resource
		if (!internalRedirect)
			m_response.targetResource = locationMatch->root + request.uri.path;
		internalRedirect = false;

		// what type is it
		FileHandler::fileType fileType = m_fileHandler.checkFileType(m_response.targetResource);
		switch (fileType) {

		case FileHandler::FileRegular:
			break;

		case FileHandler::FileDirectory:
			if (m_response.targetResource.at(m_response.targetResource.length() - 1) != '/') {
				m_response.targetResource += "/";
				m_response.status = StatusMovedPermanently;
			} else {
				m_response.targetResource += locationMatch->index;
				internalRedirect = true;
			}
			break;

		case FileHandler::FileNotExist:
			m_response.status = StatusNotFound;
			break;

		case FileHandler::FileOther:
			m_response.status = StatusForbidden;
			break;

		case FileHandler::StatError:
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
