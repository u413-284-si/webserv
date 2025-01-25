#include "test_helpers.hpp"

#include "CGIHandler.hpp"
#include "ConfigFile.hpp"
#include "MockProcessOps.hpp"
#include "MockFileSystemOps.hpp"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Contains;
using ::testing::StrEq;

class CGIHandlerTest : public ::testing::Test {
protected:
	CGIHandlerTest()
	{
		ON_CALL(processOps, pipeProcess)
			.WillByDefault(Return(0));

		m_request.uri.path = "/cgi-bin/test.py/some/more/path";
		m_request.uri.query = "name=John&age=25";
		m_request.headers["content-length"] = "20";
		m_request.headers["content-type"] = "text/plain";
		m_request.method = MethodPost;
		m_request.targetResource = "/var/www/html/cgi-bin/test.py";

		Location locationCGI;
		locationCGI.cgiExt = ".py";
		locationCGI.cgiPath = "/usr/bin/python3";
		locationCGI.path = "/cgi-bin";
		locationCGI.root = "/var/www/html";

		Location locationPath;
		locationPath.path = "/some/more";
		locationPath.root = "/another/location";

		configFile.servers[0].locations.emplace_back(locationCGI);
		configFile.servers[0].locations.emplace_back(locationPath);
		connection.location = configFile.servers[0].locations.begin() + 1;

		connections.insert(std::pair<int, Connection>(dummyFd, connection));
		cgiConnections.insert(std::pair<int, Connection*>(dummyFd, &connection));
	}
	~CGIHandlerTest() override { }

	const Socket clientSock = { "1.23.4.56", "36952" };
	const Socket serverSock = { "127.0.0.1", "8080" };
	const int dummyFd = 10;

	ConfigFile configFile = createTestConfigfile();
	Connection connection = Connection(serverSock, clientSock, dummyFd, configFile.servers);
	HTTPRequest& m_request = connection.m_request;

	NiceMock<MockProcessOps> processOps;
	MockFileSystemOps fileSystemOps;

	std::map<int, Connection> connections;
	std::map<int, Connection*> cgiConnections;

};

TEST_F(CGIHandlerTest, Ctor)
{
	// Arrange
	EXPECT_CALL(fileSystemOps, checkFileType)
		.WillOnce(Return(FileSystemOps::FileRegular));

	// Act
	CGIHandler cgiHandler(connection, processOps, fileSystemOps);
	const std::vector<std::string>& env = cgiHandler.getEnv();
	const std::vector<std::string>& argv = cgiHandler.getArgv();

	// Assert
	EXPECT_EQ(cgiHandler.getCGIPath(), "/usr/bin/python3");
	EXPECT_EQ(cgiHandler.getCGIExt(), ".py");

	EXPECT_THAT(env, Contains(StrEq("CONTENT_LENGTH=20")));
	EXPECT_THAT(env, Contains(StrEq("CONTENT_TYPE=text/plain")));
	EXPECT_THAT(env, Contains(StrEq("GATEWAY_INTERFACE=CGI/1.1")));
	EXPECT_THAT(env, Contains(StrEq("PATH_INFO=/some/more/path")));
	EXPECT_THAT(env, Contains(StrEq("PATH_TRANSLATED=/another/location/some/more/path")));
	EXPECT_THAT(env, Contains(StrEq("QUERY_STRING=name=John&age=25")));
	EXPECT_THAT(env, Contains(StrEq("REDIRECT_STATUS=200")));
	EXPECT_THAT(env, Contains(StrEq("REMOTE_ADDR=1.23.4.56")));
	EXPECT_THAT(env, Contains(StrEq("REMOTE_PORT=36952")));
	EXPECT_THAT(env, Contains(StrEq("REQUEST_METHOD=POST")));
	EXPECT_THAT(env, Contains(StrEq("REQUEST_URI=/cgi-bin/test.py/some/more/path?name=John&age=25")));
	EXPECT_THAT(env, Contains(StrEq("SCRIPT_NAME=/cgi-bin/test.py")));
	EXPECT_THAT(env, Contains(StrEq("SCRIPT_FILENAME=/var/www/html/cgi-bin/test.py")));
	EXPECT_THAT(env, Contains(StrEq("SERVER_ADDR=127.0.0.1")));
	EXPECT_THAT(env, Contains(StrEq("SERVER_NAME=127.0.0.1")));
	EXPECT_THAT(env, Contains(StrEq("SERVER_PORT=8080")));

	EXPECT_THAT(argv, Contains(StrEq("/usr/bin/python3")));
	EXPECT_THAT(argv, Contains(StrEq("/var/www/html/cgi-bin/test.py")));
}

TEST_F(CGIHandlerTest, NoPathInfo)
{
	// Arrange
	m_request.uri.path = "/cgi-bin/test.py";

	// Act
	CGIHandler cgiHandler(connection, processOps, fileSystemOps);
	const std::vector<std::string>& env = cgiHandler.getEnv();

	// Assert
	EXPECT_THAT(env, Contains(StrEq("PATH_INFO=")));
	EXPECT_THAT(env, Contains(StrEq("PATH_TRANSLATED=")));
}

TEST_F(CGIHandlerTest, PipeFail)
{
	// Arrange
	ON_CALL(processOps, pipeProcess)
		.WillByDefault(Return(-1));
	EXPECT_CALL(fileSystemOps, checkFileType)
		.WillOnce(Return(FileSystemOps::FileRegular));

	// Act
	CGIHandler cgiHandler(connection, processOps, fileSystemOps);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
}

TEST_F(CGIHandlerTest, ForkFail)
{
	// Arrange
	ON_CALL(processOps, forkProcess)
		.WillByDefault(Return(-1));
	EXPECT_CALL(fileSystemOps, checkFileType)
		.WillOnce(Return(FileSystemOps::FileRegular));

	// Act
	CGIHandler cgiHandler(connection, processOps, fileSystemOps);
	cgiHandler.execute(dummyFd, connections, cgiConnections);

	// Assert
	EXPECT_EQ(connection.m_request.httpStatus, StatusInternalServerError);
}
