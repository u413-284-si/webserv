#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "LogOstreamInserters.hpp"

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

TEST(OstreamInserters, URI)
{
	std::ostringstream ostream;
	URI uri;
	uri.path = "/path/to/resource";
	uri.query = "query=value";
	uri.fragment = "fragment";
	ostream << uri;
	const std::string expected =
	"  Path: /path/to/resource\n"
	"  Query: query=value\n"
	"  Fragment: fragment\n";
	EXPECT_EQ(ostream.str(), expected);
}

TEST(OstreamInserters, HTTPRequest)
{
	URI uri;
	uri.path = "/path/to/resource";
	uri.query = "query=value";
	uri.fragment = "fragment";

	HTTPRequest httpRequest;
	httpRequest.method = MethodGet;
	httpRequest.uri = uri;
	httpRequest.version = "HTTP/1.1";
	httpRequest.headers["Host"] = "localhost:8080";
	httpRequest.headers["User-Agent"] = "curl/7.68.0";
	httpRequest.body = "Hello, World!";
	httpRequest.httpStatus = StatusOK;
	httpRequest.shallCloseConnection = false;

	std::ostringstream ostream;
	ostream << httpRequest;

	std::ostringstream expected;
	expected <<
	"Method: GET\n"
	"URI:\n" <<
	uri <<
	"Version: HTTP/1.1\n"
	"Headers:\n"
	"  Host: localhost:8080\n"
	"  User-Agent: curl/7.68.0\n"
	"Body: Hello, World!\n"
	"HTTP status: 200 OK\n"
	"Shall close connection: 0\n";
	EXPECT_EQ(ostream.str(), expected.str());
}

TEST(OstreamInserters, HTTPResponse)
{
	LimitExcept limitExcept;
	limitExcept.allowedMethods[MethodGet] = true;
	limitExcept.allowedMethods[MethodPost] = true;
	limitExcept.allowedMethods[MethodDelete] = false;
	limitExcept.allow = "127.0.0.1";
	limitExcept.deny = "192.168.0.1";

	Location location;
	location.path = "/path/to/resource";
	location.root = "/root";
	location.index = "index.html";
	location.cgiExt = ".php";
	location.cgiPath = "/cgi-bin";
	location.isAutoindex = true;
	location.limitExcept = limitExcept;
	location.returns[StatusOK] = "OK.html";
	location.returns[StatusBadRequest] = "BadRequest.html";
	std::vector<Location> locations;
	locations.push_back(location);

	HTTPResponse httpResponse;
	httpResponse.status = StatusOK;
	httpResponse.targetResource = "/path/to/resource";
	httpResponse.body = "<html><body><h1>Hello, World!</h1></body></html>";
	httpResponse.location = locations.begin();
	httpResponse.method = MethodGet;
	httpResponse.isAutoindex = true;

	std::ostringstream ostream;
	ostream << httpResponse;

	std::ostringstream expected;
	expected <<
	"Status code: 200 OK\n"
	"Target resource: /path/to/resource\n"
	"Body: <html><body><h1>Hello, World!</h1></body></html>\n"
	"Location:\n" <<
	location <<
	"\n"
	"Method: GET\n"
	"Autoindex: 1\n";
	EXPECT_EQ(ostream.str(), expected.str());
}
