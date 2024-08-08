#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "StatusCode.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

TEST(OstreamInserters, Method)
{
	std::ostringstream ostream;
	ostream << MethodGet;
	EXPECT_EQ(ostream.str(), "GET");
	ostream.str("");
	ostream << MethodPost;
	EXPECT_EQ(ostream.str(), "POST");
	ostream.str("");
	ostream << MethodDelete;
	EXPECT_EQ(ostream.str(), "DELETE");
	ostream.str("");
	ostream << MethodCount;
	EXPECT_EQ(ostream.str(), "enum MethodCount");
}

TEST(OstreamInserters, StatusCode)
{
	std::ostringstream ostream;
	ostream << StatusOK;
	EXPECT_EQ(ostream.str(), "200 OK");
	ostream.str("");
	ostream << StatusMovedPermanently;
	EXPECT_EQ(ostream.str(), "301 Moved Permanently");
	ostream.str("");
	ostream << StatusBadRequest;
	EXPECT_EQ(ostream.str(), "400 Bad Request");
	ostream.str("");
	ostream << StatusForbidden;
	EXPECT_EQ(ostream.str(), "403 Forbidden");
	ostream.str("");
	ostream << StatusNotFound;
	EXPECT_EQ(ostream.str(), "404 Not Found");
	ostream.str("");
	ostream << StatusMethodNotAllowed;
	EXPECT_EQ(ostream.str(), "405 Method Not Allowed");
	ostream.str("");
	ostream << StatusInternalServerError;
	EXPECT_EQ(ostream.str(), "500 Internal Server Error");
	ostream.str("");
	ostream << StatusMethodNotImplemented;
	EXPECT_EQ(ostream.str(), "501 Method Not Implemented");
	ostream.str("");
	ostream << StatusNonSupportedVersion;
	EXPECT_EQ(ostream.str(), "505 HTTP Version Not Supported");
}

// sets default values for all struct which get printed
class OstreamInsertersTest : public ::testing::Test {
	protected:

	LimitExcept m_limitExcept = {
		.allowedMethods = { true, true, false },
		.allow = "127.0.0.1",
		.deny = "192.168.0.1"
	};

	Location m_location = {
		.path = "/path/to/resource",
		.root = "/root",
		.index = "index.html",
		.cgiExt = ".php",
		.cgiPath = "/cgi-bin",
		.isAutoindex = true,
		.limitExcept = m_limitExcept,
		.returns = {
			{ StatusOK, "OK.html" },
			{ StatusBadRequest, "BadRequest.html" }
		}
	};

	ServerConfig m_server = {
		.serverName = "localhost",
		.host = "10.11.12.13",
		.port = 8080,
		.maxBodySize = 1024,
		.errorPage = {
			{ StatusBadRequest, "BadRequest.html" },
			{ StatusForbidden, "Forbidden.html" }
		},
		.locations = { m_location }
	};

	ConfigFile m_configFile = {
		.serverConfigs = { m_server }
	};

	URI m_uri = {
		.path = "/path/to/resource",
		.query = "query=value",
		.fragment = "fragment"
	};

	HTTPRequest m_httpRequest = {
		.method = MethodGet,
		.uri = m_uri,
		.version = "HTTP/1.1",
		.headers = {
			{ "Host", "localhost" },
			{ "User-Agent", "curl/7.68.0" }
		},
		.body = "Hello, World!",
		.httpStatus = StatusOK,
		.shallCloseConnection = false
	};

	HTTPResponse m_httpResponse = {
		.status = StatusOK,
		.targetResource = "/path/to/resource",
		.body = "<html><body><h1>Hello, World!</h1></body></html>",
		.location = m_server.locations.begin(),
		.method = MethodGet,
		.isAutoindex = true
	};
};

TEST_F(OstreamInsertersTest, Location)
{
	std::ostringstream ostream;
	ostream << m_location;

	const std::string expected =
	"Path: /path/to/resource\n"
	"Root: /root\n"
	"Index: index.html\n"
	"CGI extension: .php\n"
	"CGI path: /cgi-bin\n"
	"Autoindex: 1\n"
	"LimitExcept:\n"
	"  Allowed methods:\n"
	"    GET\n"
	"    POST\n"
	"  Allow: 127.0.0.1\n"
	"  Deny: 192.168.0.1\n"
	"Returns:\n"
	"  200: OK.html\n"
	"  400: BadRequest.html\n";

	EXPECT_EQ(ostream.str(), expected);
}

TEST_F(OstreamInsertersTest, ServerConfig)
{
	std::ostringstream ostream;
	ostream << m_server;

	std::ostringstream expected;
	expected <<
	"Server name: localhost\n"
	"Host: 10.11.12.13\n"
	"Port: 8080\n"
	"Max body size: 1024\n"
	"Error pages:\n"
	"  400: BadRequest.html\n"
	"  403: Forbidden.html\n"
	"Locations:\n" << m_location;

	EXPECT_EQ(ostream.str(), expected.str());
}

TEST_F(OstreamInsertersTest, ConfigFile)
{
	std::ostringstream ostream;
	ostream << m_configFile;

	std::ostringstream expected;
	expected <<
	"Config file\n"
	<< m_server;
}

TEST_F(OstreamInsertersTest, URI)
{
	std::ostringstream ostream;
	ostream << m_uri;

	const std::string expected =
	"  Path: /path/to/resource\n"
	"  Query: query=value\n"
	"  Fragment: fragment\n";

	EXPECT_EQ(ostream.str(), expected);
}

TEST_F(OstreamInsertersTest, HTTPRequest)
{
	std::ostringstream ostream;
	ostream << m_httpRequest;

	std::ostringstream expected;
	expected <<
	"Method: " << m_httpRequest.method << "\n"
	"URI:\n" << m_uri <<
	"Version: HTTP/1.1\n"
	"Headers:\n"
	"  Host: localhost\n"
	"  User-Agent: curl/7.68.0\n"
	"Body: Hello, World!\n"
	"HTTP status: " << m_httpRequest.httpStatus << "\n"
	"Shall close connection: 0\n";
	EXPECT_EQ(ostream.str(), expected.str());
}

TEST_F(OstreamInsertersTest, HTTPResponse)
{
	std::ostringstream ostream;
	ostream << m_httpResponse;

	std::ostringstream expected;
	expected <<
	"Status code: " << m_httpResponse.status << "\n"
	"Target resource: /path/to/resource\n"
	"Body: <html><body><h1>Hello, World!</h1></body></html>\n"
	"Location:\n" << m_location << "\n"
	"Method: " << m_httpResponse.method << "\n"
	"Autoindex: 1\n";
	EXPECT_EQ(ostream.str(), expected.str());
}
