#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "HTTPRequest.hpp"
#include "Socket.hpp"
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
	EXPECT_EQ(ostream.str(), "200");
	ostream.str("");
	ostream << StatusMovedPermanently;
	EXPECT_EQ(ostream.str(), "301");
	ostream.str("");
	ostream << StatusBadRequest;
	EXPECT_EQ(ostream.str(), "400");
	ostream.str("");
	ostream << StatusForbidden;
	EXPECT_EQ(ostream.str(), "403");
	ostream.str("");
	ostream << StatusNotFound;
	EXPECT_EQ(ostream.str(), "404");
	ostream.str("");
	ostream << StatusMethodNotAllowed;
	EXPECT_EQ(ostream.str(), "405");
	ostream.str("");
	ostream << StatusInternalServerError;
	EXPECT_EQ(ostream.str(), "500");
	ostream.str("");
	ostream << StatusMethodNotImplemented;
	EXPECT_EQ(ostream.str(), "501");
	ostream.str("");
	ostream << StatusNonSupportedVersion;
	EXPECT_EQ(ostream.str(), "505");
}

TEST(OstreamInserters, Socket)
{
	std::ostringstream ostream;
	ostream << Socket { "127.0.0.1", "8080" };
	EXPECT_EQ(ostream.str(), "127.0.0.1:8080");
}

TEST(OstreamInserters, ConnectionStatus)
{
	std::ostringstream ostream;
	ostream << Connection::ReceiveHeader;
	EXPECT_EQ(ostream.str(), "ReceiveHeader");
	ostream.str("");
	ostream << Connection::ReceiveBody;
	EXPECT_EQ(ostream.str(), "ReceiveBody");
	ostream.str("");
	ostream << Connection::BuildResponse;
	EXPECT_EQ(ostream.str(), "BuildResponse");
	ostream.str("");
	ostream << Connection::SendResponse;
	EXPECT_EQ(ostream.str(), "SendResponse");
	ostream.str("");
	ostream << Connection::Timeout;
	EXPECT_EQ(ostream.str(), "Timeout");
	ostream.str("");
	ostream << Connection::Closed;
	EXPECT_EQ(ostream.str(), "Closed");
}

// sets default values for all struct which get printed
class OstreamInsertersTest : public ::testing::Test {
protected:
	Location m_location;
	ConfigServer m_server;
	ConfigFile m_configFile;
	Connection m_connection = Connection(Socket { "127.0.0.1", "8080" }, Socket { "1.1.1.1", "1234" }, 10, m_configFile.servers);
	URI m_uri;
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
		m_server.host = "127.0.0.1";
		m_server.port = "80";
		m_server.maxBodySize = 1024;
		m_server.errorPage = { { StatusBadRequest, "BadRequest.html" }, { StatusForbidden, "Forbidden.html" } };
		m_server.locations = { m_location };

		m_configFile.servers = { m_server };

		m_httpRequest.method = MethodGet;
		m_httpRequest.uri = m_uri;
		m_httpRequest.version = "HTTP/1.1";
		m_httpRequest.headers = { { "Host", "localhost" }, { "User-Agent", "curl/7.68.0" } };
		m_httpRequest.body = "Hello, World!";
		m_httpRequest.httpStatus = StatusOK;
		m_httpRequest.shallCloseConnection = false;

		m_connection.m_timeSinceLastEvent = 1234;
		m_connection.m_buffer = "GET / HTTP/1.1";
		m_connection.m_request = m_httpRequest;
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
								 "  200: OK.html\n"
								 "  400: BadRequest.html\n";

	EXPECT_EQ(ostream.str(), expected);
}

TEST_F(OstreamInsertersTest, ServerConfig)
{
	std::ostringstream ostream;
	ostream << m_server;

	std::ostringstream expected;
	expected << "Server name: localhost\n"
				"Host: 127.0.0.1\n"
				"Port: 80\n"
				"Max body size: 1024\n"
				"Error pages:\n"
				"  400: BadRequest.html\n"
				"  403: Forbidden.html\n"
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

TEST_F(OstreamInsertersTest, Connection)
{
	std::ostringstream ostream;
	ostream << m_connection;

	std::ostringstream expected;
	expected << "Server: " << m_connection.m_serverSocket
			 << "\n"
				"Client: "
			 << m_connection.m_clientSocket
			 << "\n"
				"Time since last event: 1234\n"
				"Status: "
			 << m_connection.m_status
			 << "\n"
				"Request:\n"
			 << m_httpRequest << "Buffer:\nGET / HTTP/1.1\n";
	EXPECT_EQ(ostream.str(), expected.str());
}