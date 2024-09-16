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
	, host("127.0.0.1")
	, port("80")
	, maxBodySize(1) // 1 MB
{
	errorPage = std::map<statusCode, std::string>();
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
	errorPage = std::map<statusCode, std::string>();
	returns = std::map<statusCode, std::string>();
	allowedMethods[0] = true;
	allowedMethods[1] = false;
	allowedMethods[2] = false;
}

/**
 * @brief Create a Dummy Config object
 *
 * @return ConfigFile The dummy config object.
 */
ConfigFile createDummyConfig()
{
	Location location1;
	location1.path = "/";
	location1.root = "/workspaces/webserv";
	location1.indices.push_back("index.htm");

	ConfigServer serverConfig8080;
	serverConfig8080.locations.push_back(location1);
	serverConfig8080.host = "127.0.0.1";
	serverConfig8080.port = "8080";
	serverConfig8080.serverName = "root";

	Location location2;
	location2.path = "/";
	location2.root = "/workspaces/webserv/doc";
	location2.indices.push_back("index.htm");

	ConfigServer serverConfig8090;
	serverConfig8090.locations.push_back(location2);
	serverConfig8090.host = "127.0.0.1";
	serverConfig8090.port = "8090";
	serverConfig8090.serverName = "doc";

	Location location3;
	location3.path = "/cgi-bin";
	location3.root = "/workspaces/webserv";
	location3.cgiPath = "/usr/bin/bash";
	location3.cgiExt = ".sh";

	Location location4;
	location4.path = "/cgi-bin";
	location4.root = "/workspaces/webserv";
	location4.cgiPath = "/usr/bin/python3";
	location4.cgiExt = ".py";

	ConfigServer serverConfig8081;
	serverConfig8081.locations.push_back(location4);
	serverConfig8081.host = "127.0.0.1";
	serverConfig8081.port = "8081";
	serverConfig8081.serverName = "cgi";

	ConfigServer serverConfig8090dupl;
	serverConfig8090dupl.locations.push_back(location2);
	serverConfig8090dupl.host = "127.0.0.1";
	serverConfig8090dupl.port = "8090";
	serverConfig8090dupl.serverName = "duplicate";

	ConfigFile configFile;
	configFile.servers.push_back(serverConfig8080);
	configFile.servers.push_back(serverConfig8090);
	configFile.servers.push_back(serverConfig8090dupl);
	configFile.servers.push_back(serverConfig8081);

	return configFile;
}
