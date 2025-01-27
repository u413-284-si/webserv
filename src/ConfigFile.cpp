#include "ConfigFile.hpp"

/* ====== CONSTRUCTORS ====== */

/**
 * @brief Construct a new Config Server:: Config Server object.
 *
 * Default constructor for the ConfigServer struct. Initializes the object with default values.
 *
 * - Sets root to "html".
 * - Sets host to "0.0.0.0"
 * - Sets port to "8080"
 * - Sets maxBodySize to 1 MB.
 * - Adds a default location with the path "/"
 */
ConfigServer::ConfigServer(void)
	: root("html")
	, host("0.0.0.0")
	, port("8080")
	, maxBodySize(constants::g_oneMegabyte)
{
	Location location;
	location.path = "/";
	locations.push_back(location);
}

/**
 * @brief Construct a new Location:: Location object
 *
 * Default constructor for the Location struct. Initializes the object with default values.
 *
 * - Sets root to "html".
 * - Sets hasAutoindex to false.
 * - Sets maxBodySize to 1 MB.
 * - Init allowMethods to true (GET), false (POST), false (DELETE)
 */
Location::Location(void)
	: root("html")
	, hasAutoindex(false)
	, maxBodySize(constants::g_oneMegabyte)
	, allowMethods()
{
	indices = std::vector<std::string>();
	errorPage = std::map<statusCode, std::string>();
	returns = std::make_pair(NoStatus, std::string());
	allowMethods[MethodGet] = true;
	allowMethods[MethodPost] = false;
	allowMethods[MethodDelete] = false;
}
