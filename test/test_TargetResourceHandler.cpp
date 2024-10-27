#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ConfigFile.hpp"
#include "Connection.hpp"
#include "MockFileSystemPolicy.hpp"
#include "TargetResourceHandler.hpp"

class TargetResourceHandlerTest : public ::testing::Test {
protected:
	TargetResourceHandlerTest()
	: m_targetResourceHandler(m_fileSystemPolicy)
	{
		m_request.method = MethodGet;
	}
	~TargetResourceHandlerTest() override { }

	ConfigFile m_configFile = createDummyConfig();
	MockFileSystemPolicy m_fileSystemPolicy;
	Socket m_serverSock = {
		.host = "127.0.0.1",
		.port = "8080"
	};
	Connection m_connection = Connection(m_serverSock, Socket(), m_configFile.servers);
	HTTPRequest& m_request = m_connection.m_request;
	TargetResourceHandler m_targetResourceHandler;
};

TEST_F(TargetResourceHandlerTest, FindCorrectLocation)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
		.WillRepeatedly(testing::Return(FileSystemPolicy::FileRegular));

	m_request.uri.path = "/index.html";
	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/index.html");
	EXPECT_EQ(m_request.httpStatus, StatusOK);
}

TEST_F(TargetResourceHandlerTest, LocationNotFound)
{
	m_configFile.servers[0].locations.clear();

	m_request.uri.path = "/something";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_request.targetResource, "");
}

TEST_F(TargetResourceHandlerTest, FileNotFound)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileNotExist));

	m_request.uri.path = "/notexist.txt";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusNotFound);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/notexist.txt");
}

TEST_F(TargetResourceHandlerTest, DirectoryRedirect)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/directory";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusMovedPermanently);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/directory");
	EXPECT_EQ(m_request.headers.at("location"), "/directory/");
}

TEST_F(TargetResourceHandlerTest, DirectoryIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType)
		.WillOnce(testing::Return(FileSystemPolicy::FileDirectory))
		.WillOnce(testing::Return(FileSystemPolicy::FileRegular));

	m_request.uri.path = "/";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/index.html");
}

TEST_F(TargetResourceHandlerTest, DirectoryAutoIndex)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/directory/";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusOK);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/directory/");
	EXPECT_TRUE(m_request.hasAutoindex);
}

TEST_F(TargetResourceHandlerTest, DirectoryForbidden)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Return(FileSystemPolicy::FileDirectory));

	m_request.uri.path = "/secret/";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusForbidden);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/secret/");
	EXPECT_FALSE(m_request.hasAutoindex);;
}

TEST_F(TargetResourceHandlerTest, ServerError)
{
	EXPECT_CALL(m_fileSystemPolicy, checkFileType).WillOnce(testing::Throw(std::runtime_error("Stat error")));

	m_request.uri.path = "/";

	m_targetResourceHandler.execute(m_connection, m_request);

	EXPECT_EQ(m_request.httpStatus, StatusInternalServerError);
	EXPECT_EQ(m_request.targetResource, "/workspaces/webserv/html/");
	EXPECT_FALSE(m_request.hasAutoindex);;
}
