#include "HTTPResponse.hpp"
#include "RequestParser.hpp"
#include "TargetResourceHandler.hpp"

TargetResourceHandler::TargetResourceHandler(const std::vector<Location>& locations, const FileHandler& fileHandler)
	: m_locations(locations)
	, m_fileHandler(fileHandler)
{
}

HTTPResponse TargetResourceHandler::execute(const HTTPRequest& request)
{
	// Check which location block matches the path
	std::vector<Location>::const_iterator locationMatch = matchLocation(request.uri.path);

	// No location found > do we also set a default location to not make extra check?
	if (locationMatch == m_locations.end())
	{
		m_response.status = StatusNotFound;
		return m_response;
	}
	m_response.targetResource = locationMatch->root + request.uri.path;
	if (m_fileHandler.isDirectory(m_response.targetResource))
	{
		if (m_response.targetResource.at(m_response.targetResource.length() - 1) != '/')
		{
			m_response.targetResource += "/";
			m_response.status = StatusMovedPermanently;
			return m_response;
		}
		m_response.targetResource += locationMatch->index;
		if (!m_fileHandler.isExistingFile(m_response.targetResource))
		{
			m_response.status = StatusForbidden;
			return m_response;
		}
	}
	return m_response;
}
