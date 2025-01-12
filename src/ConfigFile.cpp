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
 * - Init allowedMethods to true (GET), false (POST), false (DELETE)
 */
Location::Location(void)
	: root("html")
	, hasAutoindex(false)
	, maxBodySize(constants::g_oneMegabyte)
	, allowedMethods()
{
	indices = std::vector<std::string>();
	errorPage = std::map<statusCode, std::string>();
	returns = std::make_pair(NoStatus, std::string());
	allowedMethods[MethodGet] = true;
	allowedMethods[MethodPost] = false;
	allowedMethods[MethodDelete] = false;
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
	location2.hasAutoindex = true;

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
	location5.allowedMethods[MethodPost] = true;

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
	location7.allowedMethods[MethodDelete] = true;

	Location location8;
	location8.path = "/redirect";
	location8.returns = std::make_pair(StatusMovedPermanently, "/secret");

	Location location9;
	location9.path = "/strange";
	location9.returns = std::make_pair(StatusRequestHeaderFieldsTooLarge, "Return Message");

	Location location10;
	location10.path = "/another";
	location10.returns = std::make_pair(StatusForbidden, "");
	location10.errorPage[StatusForbidden] = "/strange";

	Location location11;
	location11.path = "/alias/";
	location11.root = "/workspaces/webserv/html";
	location11.alias = "/workspaces/webserv/html/images/";

	ConfigServer serverConfig8080;
	serverConfig8080.locations.clear();
	serverConfig8080.locations.push_back(location1);
	serverConfig8080.locations.push_back(location2);
	serverConfig8080.locations.push_back(location3);
	serverConfig8080.locations.push_back(location4);
	serverConfig8080.locations.push_back(location5);
	serverConfig8080.locations.push_back(location7);
	serverConfig8080.locations.push_back(location8);
	serverConfig8080.locations.push_back(location9);
	serverConfig8080.locations.push_back(location10);
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
