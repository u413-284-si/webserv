#include "ConfigFile.hpp"
#include <string>

/* ====== CONSTRUCTORS ====== */

/**
 * @brief Construct a new Config Server:: Config Server object.
 *
 * Default constructor for the ConfigServer struct. Initializes the object with default values.
 *
 * - Sets the root directory to "html".
 * - Sets the maximum body size to 1.
 * - Sets the error pages to an empty map.
 * - Initializes the listen map with "127.0.0.1" and port 80.
 * - Sets the locations to an empty vector.
 */
ConfigServer::ConfigServer(void)
	: m_root("html")
	, m_maxBodySize(1)
{
	m_listen.insert(std::make_pair("127.0.0.1", 80));
	m_errorPage = std::map<unsigned short, std::string>();
	m_locations = std::vector<Location>();
}

/**
 * @brief Construct a new Location:: Location object
 *
 * Default constructor for the Location struct. Initializes the object with default values.
 *
 * - Sets the root directory to "html".
 * - Sets the autoindex flag to false.
 * - Sets the maximum body size to 1.
 * - Initializes the allowed methods vector to contain false for all methods.
 * - Initializes the indices vector to be empty.
 * - Initializes the returns map to be empty.
 */
Location::Location(void)
	: m_root("html")
	, m_isAutoindex(false)
	, m_maxBodySize(1)
	, m_allowedMethods()
{
	m_indices = std::vector<std::string>();
	m_returns = std::map<unsigned short, std::string>();
	m_allowedMethods[0] = true;
	m_allowedMethods[1] = false;
	m_allowedMethods[2] = false;
}

/* ====== GETTERS ====== */

/**
 * @brief Gets the server name.
 *
 * @return std::string Server name.
 */
std::string ConfigServer::getServerName() const { return m_serverName; }

/**
 * @brief Gets the root path.
 *
 * @return std::string Root path.
 */
std::string ConfigServer::getRoot() const { return m_root; }

/**
 * @brief Gets the listen values.
 *
 * Each element in the map is a pair of IP setress and port number.
 *
 * @return std::map<std::string, unsigned short> Listen values
 */
std::map<std::string, unsigned short> ConfigServer::getListen() const { return m_listen; }

/**
 * @brief Gets the maximum body size
 *
 * @return unsigned long Maximum body size
 */
unsigned long ConfigServer::getMaxBodySize() const { return m_maxBodySize; }

/**
 * @brief Gets the error pages
 *
 * Each element in the map is a pair of error code and path for the error page.
 *
 * @return std::map<unsigned short, std::string> Error pages
 */
std::map<unsigned short, std::string> ConfigServer::getErrorPages() const { return m_errorPage; }

/**
 * @brief Gets the locations
 *
 * @return std::vector<Location> Locations
 */
std::vector<Location> ConfigServer::getLocations() const { return m_locations; }

/**
 * @brief Gets the path defined for the location
 *
 * @return std::string Path defined for the location
 */
std::string Location::getPath() const { return m_path; }

/**
 * @brief Gets the root path.
 *
 * @return std::string Root path.
 */
std::string Location::getRoot() const { return m_root; }

/**
 * @brief Gets the indices defined for the location.
 *
 * @return std::vector<std::string> Indices defined for the location.
 */
std::vector<std::string> Location::getIndices() const { return m_indices; }

/**
 * @brief Gets the cgi extension.
 *
 * @return std::string Cgi extension.
 */
std::string Location::getCgiExt() const { return m_cgiExt; }

/**
 * @brief Gets the cgi path.
 *
 * @return std::string Cgi path.
 */
std::string Location::getCgiPath() const { return m_cgiPath; }

/**
 * @brief Gets the autoindex flag.
 *
 * @return true If the autoindex flag is set.
 * @return false If the autoindex flag is not set.
 */
bool Location::getIsAutoindex() const { return m_isAutoindex; }

/**
 * @brief Gets the maximum body size.
 *
 * @return unsigned long Maximum body size.
 */
unsigned long Location::getMaxBodySize() const { return m_maxBodySize; }

/**
 * @brief Gets the error codes with the corresponding error pages.
 *
 * @return std::map<unsigned short, std::string> Error codes with the corresponding error pages
 */
std::map<unsigned short, std::string> Location::getErrorPages() const { return m_errorPage; }

/**
 * @brief Gets the value of the allowed methods
 *
 * @param method The method to check
 * @return true If the method is allowed
 * @return false If the method is not allowed
 */
bool Location::getAllowedMethod(Method method) const
{
	switch (method) {
	case MethodGet:
		return m_allowedMethods[0];
	case MethodPost:
		return m_allowedMethods[1];
	case MethodDelete:
		return m_allowedMethods[2];
	default:
		return false;
	}
}

/**
 * @brief Returns the error code with the correspoding redirection URL.
 *
 * @return std::map<unsigned short, std::string> Error codes with the corresponding redirection URLs.
 */
std::map<unsigned short, std::string> Location::getReturns() const { return m_returns; }

/* ====== SETTERS ====== */

/**
 * @brief Sets the root path.
 *
 * @param root The root path
 */
void ConfigServer::setRoot(const std::string& root) { m_root = root; }

/**
 * @brief Sets listen value (ip setress and port).
 *
 * @param ipsetress The ip setress.
 * @param port The port number.
 */
void ConfigServer::setListen(const std::string& ipsetress, unsigned short port)
{
	m_listen.insert(std::make_pair(ipsetress, port));
}

/**
 * @brief Sets the maximum body size.
 *
 * @param maxBodySize  The maximum body size.
 */
void ConfigServer::setMaxBodySize(unsigned long maxBodySize) { m_maxBodySize = maxBodySize; }

/**
 * @brief Sets an error code with the corresponding error page.
 *
 * @param code The error code
 * @param path The error page
 */
void ConfigServer::setErrorPage(unsigned short code, const std::string& path)
{
	m_errorPage.insert(std::make_pair(code, path));
}

/**
 * @brief Sets a location
 *
 * @param location Location
 */
void ConfigServer::setLocation(const Location& location) { m_locations.push_back(location); }

/**
 * @brief Sets the path defined for the location.
 *
 * @param path The path defined for the location.
 */
void Location::setPath(const std::string& path) { m_path = path; }

/**
 * @brief Sets the root path.
 *
 * @param root The root path.
 */
void Location::setRoot(const std::string& root) { m_root = root; }

/**
 * @brief Sets the indices defined for the location.
 *
 * @param indices The indices defined for the location.
 */
void Location::setIndex(const std::string& index) { m_indices.push_back(index); }

/**
 * @brief Sets the cgi extension.
 *
 * @param cgiExt The cgi extension.
 */
void Location::setCgiExt(const std::string& cgiExt) { m_cgiExt = cgiExt; }

/**
 * @brief Sets the cgi path.
 *
 * @param cgiPath The cgi path.
 */
void Location::setCgiPath(const std::string& cgiPath) { m_cgiPath = cgiPath; }

/**
 * @brief Sets the autoindex flag.
 *
 * @param isAutoindex The autoindex flag.
 */
void Location::setIsAutoindex(bool isAutoindex) { m_isAutoindex = isAutoindex; }

/**
 * @brief Sets the maximum body size.
 *
 * @param maxBodySize The maximum body size.
 */
void Location::setMaxBodySize(unsigned long maxBodySize) { m_maxBodySize = maxBodySize; }

/**
 * @brief Sets an error code with the corresponding error page.
 *
 * @param code The error code.
 * @param path The error page.
 */
void Location::setErrorPage(unsigned short code, const std::string& path)
{
	m_errorPage.insert(std::make_pair(code, path));
}
