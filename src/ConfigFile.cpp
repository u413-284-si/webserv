#include "ConfigFile.hpp"

/* ====== CONSTRUCTORS ====== */

/**
 * @brief Construct a new Config Server:: Config Server object.
 *
 * Default constructor for the ConfigServer struct. Initializes the object with default values.
 *
 * - Sets the root directory to "html".
 * - Sets the maximum body size to 1 (1 MB).
 * - Sets the error pages to an empty map.
 * - Initializes the listen map with "0.0.0.0" and port "8080".
 * - Adds a default location with the path "/"
 */
ConfigServer::ConfigServer(void)
	: root("html")
	, host("0.0.0.0")
	, port("8080")
	, maxBodySize(1) // 1 MB
{
	errorPage = std::map<statusCode, std::string>();
	Location location;
	location.path = "/";
	locations.push_back(location);
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
	, alias(std::pair<bool, std::string>(false, ""))
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
	location1.root = "/workspaces/webserv/html";
	location1.indices.push_back("index.html");
	location1.errorPage[StatusNotFound] = "/error404.html";

	Location location2;
	location2.path = "/directory/";
	location2.root = "/workspaces/webserv/html";
	location2.isAutoindex = true;

	Location location3;
	location3.path = "/error";
	location3.root = "/workspaces/webserv/html/error";

	Location location4;
	location4.path = "/secret/";
	location4.root = "/workspaces/webserv/html";
	location4.errorPage[StatusForbidden] = "/error403.html";

	Location location5;
	location5.path = "/cgi-bin";
	location5.root = "/workspaces/webserv";
	location5.cgiPath = "/usr/bin/bash";
	location5.cgiExt = ".sh";

	Location location6;
	location6.path = "/cgi-bin";
	location6.root = "/workspaces/webserv";
	location6.cgiPath = "/usr/bin/python3";
	location6.cgiExt = ".py";
	location6.allowedMethods[MethodGet] = true;
	location6.allowedMethods[MethodPost] = true;

	Location location7;
	location7.path = "/uploads/";
	location7.root = "/workspaces/webserv/html";
	location7.allowedMethods[MethodPost] = true;

	Location location11;
	location11.path = "/alias/";
	location11.root = "/workspaces/webserv/html";
	location11.alias = std::make_pair(true, "/workspaces/webserv/html/images/");

	ConfigServer serverConfig8080;
	serverConfig8080.locations.clear();
	serverConfig8080.locations.push_back(location1);
	serverConfig8080.locations.push_back(location2);
	serverConfig8080.locations.push_back(location3);
	serverConfig8080.locations.push_back(location4);
	serverConfig8080.locations.push_back(location5);
	serverConfig8080.locations.push_back(location7);
	serverConfig8080.locations.push_back(location11);
	serverConfig8080.host = "127.0.0.1";
	serverConfig8080.port = "8080";
	serverConfig8080.serverName = "default";

	ConfigServer serverConfig8090;
	serverConfig8090.locations.clear();
	serverConfig8090.locations.push_back(location1);
	serverConfig8090.host = "127.0.0.1";
	serverConfig8090.port = "8090";
	serverConfig8090.serverName = "doc";

	ConfigServer serverConfig8081;
	serverConfig8081.locations.clear();
	serverConfig8081.locations.push_back(location6);
	serverConfig8081.host = "127.0.0.1";
	serverConfig8081.port = "8081";
	serverConfig8081.serverName = "cgi";

	ConfigServer serverConfig8082;
	serverConfig8082.locations.push_back(location7);
	serverConfig8082.host = "127.0.0.1";
	serverConfig8082.port = "8082";
	serverConfig8082.serverName = "post";

	ConfigServer serverConfig8090dupl;
	serverConfig8090dupl.locations.clear();
	serverConfig8090dupl.locations.push_back(location1);
	serverConfig8090dupl.host = "127.0.0.1";
	serverConfig8090dupl.port = "8090";
	serverConfig8090dupl.serverName = "duplicate";

	ConfigFile configFile;
	configFile.servers.push_back(serverConfig8080);
	configFile.servers.push_back(serverConfig8090);
	configFile.servers.push_back(serverConfig8090dupl);
	configFile.servers.push_back(serverConfig8081);
	configFile.servers.push_back(serverConfig8082);

	return configFile;
}
