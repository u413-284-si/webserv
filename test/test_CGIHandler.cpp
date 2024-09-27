#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "CGIHandler.hpp"
#include "ConfigFile.hpp"
#include "HTTPRequest.hpp"
#include "Method.hpp"
#include "MockProcessOps.hpp"
#include "ProcessOps.hpp"
#include "Socket.hpp"

using ::testing::NiceMock;
using ::testing::Return;

class CGIHandlerTest : public ::testing::Test {
protected:
	CGIHandlerTest()
	{
		ON_CALL(processOps, pipeProcess)
			.WillByDefault(Return(0));

		request.uri.path = "/cgi-bin/test.py/some/more/path";
		request.uri.query = "name=John&age=25";
		request.headers["Content-Length"] = "20";
		request.headers["Content-Type"] = "text/plain";
		request.method = MethodPost;
		connection.m_request = request;

		location.cgiExt = ".py";
		location.cgiPath = "/usr/bin/python3";
		location.path = "/cgi-bin";
		location.root = "/var/www/html";

		locations.push_back(location);
		connection.location = locations.begin();

		serverConfig.host = serverSock.host;
		serverConfig.port = serverSock.port;
		configFile.servers.push_back(serverConfig);
	}
	~CGIHandlerTest() override { }

	const Socket clientSock = { "1.23.4.56", "36952" };
	const Socket serverSock = { "127.0.0.1", "8080" };
	const int dummyFd = 10;
	HTTPRequest request;
	Location location;
	std::vector<Location> locations;
	
	ConfigServer serverConfig;
	ConfigFile configFile;
	Connection connection = Connection(serverSock, clientSock, dummyFd, configFile.servers);
	NiceMock<MockProcessOps> processOps;
};

TEST_F(CGIHandlerTest, Ctor)
{
	// Arrange

	// Act
	CGIHandler cgiHandler(connection, processOps);
	const std::vector<std::string> env = cgiHandler.getEnv();
	const std::vector<std::string> argv = cgiHandler.getArgv();

	// Assert
	EXPECT_EQ(cgiHandler.getCGIPath(), "/usr/bin/python3");
	EXPECT_EQ(cgiHandler.getCGIExt(), ".py");

	EXPECT_EQ(*std::find(env.begin(), env.end(), "CONTENT_LENGTH=20"), "CONTENT_LENGTH=20");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "CONTENT_TYPE=text/plain"), "CONTENT_TYPE=text/plain");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "GATEWAY_INTERFACE=CGI/1.1"), "GATEWAY_INTERFACE=CGI/1.1");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "PATH_INFO=/some/more/path"), "PATH_INFO=/some/more/path");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "PATH_TRANSLATED=/var/www/html/some/more/path"), "PATH_TRANSLATED=/var/www/html/some/more/path");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "QUERY_STRING=name=John&age=25"), "QUERY_STRING=name=John&age=25");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "REDIRECT_STATUS=200"), "REDIRECT_STATUS=200");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "REMOTE_ADDR=1.23.4.56"), "REMOTE_ADDR=1.23.4.56");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "REMOTE_PORT=36952"), "REMOTE_PORT=36952");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "REQUEST_METHOD=POST"), "REQUEST_METHOD=POST");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "REQUEST_URI=/cgi-bin/test.py/some/more/path?name=John&age=25"), "REQUEST_URI=/cgi-bin/test.py/some/more/path?name=John&age=25");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SCRIPT_NAME=/cgi-bin/test.py"), "SCRIPT_NAME=/cgi-bin/test.py");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SCRIPT_FILENAME=/var/www/html/cgi-bin/test.py"), "SCRIPT_FILENAME=/var/www/html/cgi-bin/test.py");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SERVER_ADDR=127.0.0.1"), "SERVER_ADDR=127.0.0.1");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SERVER_NAME=127.0.0.1"), "SERVER_NAME=127.0.0.1");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SERVER_PORT=8080"), "SERVER_PORT=8080");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "SYSTEM_ROOT=/var/www/html"), "SYSTEM_ROOT=/var/www/html");

	EXPECT_EQ(*std::find(argv.begin(), argv.end(), "/usr/bin/python3"), "/usr/bin/python3");
	EXPECT_EQ(*std::find(argv.begin(), argv.end(), "/var/www/html/cgi-bin/test.py"), "/var/www/html/cgi-bin/test.py");
}

TEST_F(CGIHandlerTest, NoPathInfo)
{
	// Arrange
	request.uri.path = "/cgi-bin/test.py";
	connection.m_request = request;

	// Act
	CGIHandler cgiHandler(connection, processOps);
	const std::vector<std::string> env = cgiHandler.getEnv();

	// Assert
	EXPECT_EQ(*std::find(env.begin(), env.end(), "PATH_INFO="), "PATH_INFO=");
	EXPECT_EQ(*std::find(env.begin(), env.end(), "PATH_TRANSLATED=/var/www/html"), "PATH_TRANSLATED=/var/www/html");
}

TEST_F(CGIHandlerTest, PipeFail)
{
	// Arrange
	ON_CALL(processOps, pipeProcess)
		.WillByDefault(Return(-1));

	// Act
	CGIHandler cgiHandler(connection, processOps);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
}

TEST_F(CGIHandlerTest, ForkFail)
{
	// Arrange
	ON_CALL(processOps, forkProcess)
		.WillByDefault(Return(-1));
	
	// Act
	CGIHandler cgiHandler(connection, processOps);
	cgiHandler.execute(connection.m_request, connection.location, processOps);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
}
