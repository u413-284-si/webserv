#include "ConfigFile.hpp"
#include <string>

/* ====== CONSTRUCTORS ====== */

/**
 * @brief Construct a new Config Server:: Config Server object.
 *
 * Default constructor for the ConfigServer struct. Initializes the object with default values.
 *
 * - Sets the root directory to "html".
 * - Sets the maximum body size to 1 (1 MB).
 * - Sets the error pages to an empty map.
 * - Initializes the listen map with "127.0.0.1" and port 80.
 * - Sets the locations to an empty vector.
 */
ConfigServer::ConfigServer(void)
	: root("html")
	, maxBodySize(1) // 1 MB
{
	listen.insert(std::make_pair("127.0.0.1", "80"));
	errorPage = std::map<unsigned short, std::string>();
	locations = std::vector<Location>();
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
	: root("html")
	, isAutoindex(false)
	, maxBodySize(1)
	, allowedMethods()
{
	indices = std::vector<std::string>();
	returns = std::map<unsigned short, std::string>();
	allowedMethods[0] = true;
	allowedMethods[1] = false;
	allowedMethods[2] = false;
}
