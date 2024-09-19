#include <gtest/gtest.h>
#include <string>

#include "CGIHandler.hpp"
#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Method.hpp"
#include "Socket.hpp"

class CGIHandlerTest : public ::testing::Test {
protected:
	CGIHandlerTest()
		: m_cgiHandler(m_cgiPath, m_cgiExt)
	{
		request.uri.path = "/cgi-bin/test.py/some/more/path";
		request.uri.query = "name=John&age=25";
		request.headers["Content-Length"] = "20";
		request.headers["Content-Type"] = "text/plain";
		request.method = MethodPost;

		location.cgiExt = ".py";
		location.cgiPath = "/usr/bin/python3";
		location.path = "/cgi-bin";
		location.root = "/var/www/html";

		locations.push_back(location);
		iter = locations.begin();
	}
	~CGIHandlerTest() override { }

	std::string m_cgiPath = "/usr/bin/python3";
	std::string m_cgiExt = ".py";
	const Socket clientSock = { "1.23.4.56", "36952" };
	const Socket serverSock = { "127.0.0.1", "8080" };
	HTTPRequest request;
	Location location;
	std::vector<Location> locations;
	std::vector<Location>::const_iterator iter;

	CGIHandler m_cgiHandler;
};

TEST_F(CGIHandlerTest, Init)
{
	// Arrange

	// Act
	m_cgiHandler.init(clientSock, serverSock, request, iter);

	// Assert
	EXPECT_EQ(m_cgiHandler.getCGIPath(), "/usr/bin/python3");
	EXPECT_EQ(m_cgiHandler.getCGIExt(), ".py");
	EXPECT_EQ(m_cgiHandler.getEnv().at("CONTENT_LENGTH"), "20");
	EXPECT_EQ(m_cgiHandler.getEnv().at("CONTENT_TYPE"), "text/plain");
	EXPECT_EQ(m_cgiHandler.getEnv().at("PATH_INFO"), "/some/more/path");
	EXPECT_EQ(m_cgiHandler.getEnv().at("PATH_TRANSLATED"), "/var/www/html/some/more/path");
	EXPECT_EQ(m_cgiHandler.getEnv().at("QUERY_STRING"), "name=John&age=25");
	EXPECT_EQ(m_cgiHandler.getEnv().at("REMOTE_ADDR"), "1.23.4.56");
	EXPECT_EQ(m_cgiHandler.getEnv().at("REMOTE_PORT"), "36952");
	EXPECT_EQ(m_cgiHandler.getEnv().at("REQUEST_METHOD"), "POST");
	EXPECT_EQ(m_cgiHandler.getEnv().at("REQUEST_URI"), "/cgi-bin/test.py/some/more/path?name=John&age=25");
	EXPECT_EQ(m_cgiHandler.getEnv().at("SCRIPT_NAME"), "/cgi-bin/test.py");
	EXPECT_EQ(m_cgiHandler.getEnv().at("SCRIPT_FILENAME"), "/var/www/html/cgi-bin/test.py");
	EXPECT_EQ(m_cgiHandler.getEnv().at("SERVER_ADDR"), "127.0.0.1");
	EXPECT_EQ(m_cgiHandler.getEnv().at("SERVER_PORT"), "8080");
	EXPECT_EQ(m_cgiHandler.getEnv().at("SYSTEM_ROOT"), "/var/www/html");
}

TEST_F(CGIHandlerTest, NoPathInfo)
{
	// Arrange
	request.uri.path = "/cgi-bin/test.py";
	// Act
	m_cgiHandler.init(clientSock, serverSock, request, iter);

	// Assert
	EXPECT_EQ(m_cgiHandler.getEnv().at("PATH_INFO"), "");
	EXPECT_EQ(m_cgiHandler.getEnv().at("PATH_TRANSLATED"), "/var/www/html");
}

TEST_F(CGIHandlerTest, ExecutePipeCreation)
{
	// Arrange

	// Act
	m_cgiHandler.init(clientSock, serverSock, request, iter);
	m_cgiHandler.execute(request, iter);

	// Assert
	EXPECT_EQ(m_cgiHandler.getPipeInWriteEnd(), 4);
	EXPECT_EQ(m_cgiHandler.getPipeOutReadEnd(), 5);
}
