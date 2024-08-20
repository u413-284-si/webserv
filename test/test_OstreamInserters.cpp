#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StatusCode.hpp"

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
	Location m_location;
	ConfigServer m_server;
	ConfigFile m_configFile;
	URI m_uri;
	HTTPResponse m_httpResponse;
	HTTPRequest m_httpRequest;

	OstreamInsertersTest()
	{
		m_location.path = "/path/to/resource";
		m_location.root = "/root";
		m_location.indices = { "index.html" };
		m_location.cgiExt = ".php";
		m_location.cgiPath = "/cgi-bin";
		m_location.isAutoindex = true;
		m_location.allowedMethods[0] = true;
		m_location.allowedMethods[1] = false;
		m_location.allowedMethods[2] = false;
		m_location.returns = { { StatusOK, "OK.html" }, { StatusBadRequest, "BadRequest.html" } };

		m_uri.path = "/path/to/resource";
		m_uri.query = "query=value";
		m_uri.fragment = "fragment";

		m_server.serverName = "localhost";
		m_server.listen = { { "127.0.0.1", "80" } };
		m_server.maxBodySize = 1024;
		m_server.errorPage = { { StatusBadRequest, "BadRequest.html" }, { StatusForbidden, "Forbidden.html" } };
		m_server.locations = { m_location };

		m_configFile.servers = { m_server };

		m_httpResponse.status = StatusOK;
		m_httpResponse.targetResource = "/path/to/resource";
		m_httpResponse.body = "<html><body><h1>Hello, World!</h1></body></html>";
		m_httpResponse.location = m_server.locations.begin();
		m_httpResponse.method = MethodGet;
		m_httpResponse.isAutoindex = true;

		m_httpRequest.method = MethodGet;
		m_httpRequest.uri = m_uri;
		m_httpRequest.version = "HTTP/1.1";
		m_httpRequest.headers = { { "Host", "localhost" }, { "User-Agent", "curl/7.68.0" } };
		m_httpRequest.body = "Hello, World!";
		m_httpRequest.httpStatus = StatusOK;
		m_httpRequest.shallCloseConnection = false;
	}
};

TEST_F(OstreamInsertersTest, Location)
{
	std::ostringstream ostream;
	ostream << m_location;

	const std::string expected = "Path: /path/to/resource\n"
								 "Root: /root\n"
								 "Indices: \n"
								 "  index.html\n"
								 "CGI extension: .php\n"
								 "CGI path: /cgi-bin\n"
								 "Autoindex: 1\n"
								 "Allowed methods:\n"
								 "  GET: 1\n"
								 "  POST: 0\n"
								 "  DELETE: 0\n"
								 "Returns:\n"
								 "  200 OK: OK.html\n"
								 "  400 Bad Request: BadRequest.html\n";

	EXPECT_EQ(ostream.str(), expected);
}

TEST_F(OstreamInsertersTest, ServerConfig)
{
	std::ostringstream ostream;
	ostream << m_server;

	std::ostringstream expected;
	expected << "Server name: localhost\n"
				"Listen: \n"
				"  127.0.0.1:80\n"
				"Max body size: 1024\n"
				"Error pages:\n"
				"  400 Bad Request: BadRequest.html\n"
				"  403 Forbidden: Forbidden.html\n"
				"Locations:\n"
			 << m_location;

	EXPECT_EQ(ostream.str(), expected.str());
}

TEST_F(OstreamInsertersTest, ConfigFile)
{
	std::ostringstream ostream;
	ostream << m_configFile;

	std::ostringstream expected;
	expected << "Config file\n" << m_server;
}

TEST_F(OstreamInsertersTest, URI)
{
	std::ostringstream ostream;
	ostream << m_uri;

	const std::string expected = "  Path: /path/to/resource\n"
								 "  Query: query=value\n"
								 "  Fragment: fragment\n";

	EXPECT_EQ(ostream.str(), expected);
}

TEST_F(OstreamInsertersTest, HTTPRequest)
{
	std::ostringstream ostream;
	ostream << m_httpRequest;

	std::ostringstream expected;
	expected << "Method: " << m_httpRequest.method
			 << "\n"
				"URI:\n"
			 << m_uri
			 << "Version: HTTP/1.1\n"
				"Headers:\n"
				"  Host: localhost\n"
				"  User-Agent: curl/7.68.0\n"
				"Body: Hello, World!\n"
				"HTTP status: "
			 << m_httpRequest.httpStatus
			 << "\n"
				"Shall close connection: 0\n";
	EXPECT_EQ(ostream.str(), expected.str());
}

TEST_F(OstreamInsertersTest, HTTPResponse)
{
	std::ostringstream ostream;
	ostream << m_httpResponse;

	std::ostringstream expected;
	expected << "Status code: " << m_httpResponse.status
			 << "\n"
				"Target resource: /path/to/resource\n"
				"Body: <html><body><h1>Hello, World!</h1></body></html>\n"
				"Location:\n"
			 << m_location
			 << "\n"
				"Method: "
			 << m_httpResponse.method
			 << "\n"
				"Autoindex: 1\n";
	EXPECT_EQ(ostream.str(), expected.str());
}
